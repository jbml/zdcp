#define AP_DEBUG

#include "zdcp.h"
#include "assert.h"
#include "httpd.h"
#include "http_config.h"
#include "ap_config.h"
#include "util_filter.h"
#include "http_log.h"
#include "util_script.h"
#include "apr_strings.h"
#include "util_script.h"
#include "ap_mpm.h"
#include "mod_zdcp.h"
#include "apr_lib.h"

#define dprintf apr_psprintf
#define DEBUG
#ifdef DEBUG
#define DDD(x) ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, r->server, "%s", (x));
#else
#define DDD(X) ;
#endif

typedef struct _conf_list_entry {
    APR_RING_ENTRY(_conf_list_entry) link;
    zdcp_conf_t *conf;
} _conf_list_entry_t;

module AP_MODULE_DECLARE_DATA zdcp_module;

enum {
    DEFAULT_ZDCP_ENABLE_VALUE = 0,
    ZDCP_ENABLE_UNSET         = -1
};

static APR_RING_ENTRY(_conf_list_entry) global_conf_list;

static apr_table_t* process_args(request_rec *r);

static char* read_table(request_rec *r, char *buf, apr_table_t *table) {
    char *l, *w, *eol;
    for (w = buf; *w != '\0'; w = eol + 1) {
        if ((eol = strstr(w, CRLF)) != NULL) {
            *eol++ = '\0';
            *eol = '\0';
        } else {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
              "zdcp: Bad header from the cache: missing terminal newline: %s", w);
            return NULL;
        }
        if (*w == '\0') { /* found the terminal CRLF where w == eol - 1 */
            //DDD("Done reading table")
            return eol + 1;
        }

        if (!(l = strchr(w, ':'))) {
            /* ignore loudly */
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
              "zdcp: Ignored bad header from the cache: %s", w);
        } else {
            *l++ = '\0';
            while (*l && apr_isspace(*l)) {
                ++l;
            }
            //DDD(apr_psprintf(r->pool, "header: [%s] [%s]", w, l))
            apr_table_add(table, w, l);
        }
    }
    DDD("Done reading table")
    return w;
}

/*
static apr_status_t free_pointer (void* p) {
    free(p);
    return APR_SUCCESS;
}
*/

static int process_post_request(request_rec *r, zdcp_conf_t *conf) {
    char *buf, *cur;
    apr_bucket_brigade *bb;
    int seen_eos = 0;
    apr_status_t rc;
    const char *cl;
    apr_size_t size = 0;
    apr_size_t count = 0;
    apr_table_t *input_table;

    cl = apr_table_get(r->headers_in, "Content-Length");
    if (cl == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                  "No Content-Length specified in the request header.");
        return HTTP_BAD_REQUEST;
    }
    size = apr_atoi64(cl);

    buf = apr_pcalloc(r->pool, size + 1);
    if (buf == NULL) {
        return HTTP_INTERNAL_SERVER_ERROR;
    }
    cur = buf;

    bb = apr_brigade_create(r->pool, r->connection->bucket_alloc);
    do {
        apr_bucket *b;
        rc = ap_get_brigade(r->input_filters, bb, AP_MODE_READBYTES,
                APR_BLOCK_READ, HUGE_STRING_LEN);
        if (rc != APR_SUCCESS) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, rc, r,
                          "Error reading PUT request entity data");
            apr_brigade_destroy(bb);
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        for (b = APR_BRIGADE_FIRST(bb);
                b != APR_BRIGADE_SENTINEL(bb);
                b = APR_BUCKET_NEXT(b)) {
            const char *data;
            apr_size_t len;

            if (APR_BUCKET_IS_EOS(b)) {
                seen_eos = 1;
                break;
            }

            /* We can't do much with this. */
            if (APR_BUCKET_IS_FLUSH(b)) {
                continue;
            }

            if (b->length == 0) {
                continue;
            }
            rc = apr_bucket_read(b, &data, &len, APR_BLOCK_READ);
            if (rc != APR_SUCCESS) {
                apr_brigade_destroy(bb);
                return HTTP_INTERNAL_SERVER_ERROR;
            }
            if (count + len > size) {
                len = size - count;
                memcpy(cur, data, len);
                count += len;
                seen_eos = 1;
                cur += len;
                break;
            } else {
                memcpy(cur, data, len);
                count += len;
                cur += len;
            }
            AP_DEBUG_ASSERT(count <= size);
        }
        apr_brigade_cleanup(bb);
    } while (!seen_eos);

    if (!seen_eos && count == 0) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
            "No content part found for the POST request.");
        return HTTP_BAD_REQUEST;
    }

    cur = buf;
    DDD(dprintf(r->pool, "strlen(content): %" APR_SIZE_T_FMT, strlen(buf)));
    input_table = apr_table_make(r->pool, 2);
    cur = read_table(r, cur, input_table);
    if (cur == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
            "Syntax error in the POST content.");
        return HTTP_BAD_REQUEST;
    }

    {
        const char *params;
        struct iovec *iov;
        const apr_table_entry_t *elts;
        apr_size_t nelts, nvec;
        int rv;
        int i, k;
        const char *cat;

        elts = (const apr_table_entry_t *)apr_table_elts(input_table)->elts;
        nelts = apr_table_elts(input_table)->nelts;
        nvec = nelts * 5;
        iov = apr_palloc(r->pool, sizeof(struct iovec) * nvec);

        for (i = 0, k = 0; i < nelts; i++) {
            if (elts[i].key != NULL) {
                iov[k].iov_base = "<";
                iov[k].iov_len  = sizeof("<") - 1;
                k++;
                iov[k].iov_base = elts[i].key;
                iov[k].iov_len  = strlen(elts[i].key);
                k++;
                iov[k].iov_base = ": ";
                iov[k].iov_len  = sizeof(": ") - 1;
                k++;
                iov[k].iov_base = elts[i].val;
                iov[k].iov_len  = strlen(elts[i].val);
                k++;
                iov[k].iov_base = ">";
                iov[k].iov_len  = sizeof(">") - 1;
                k++;
            }
        }
        params = apr_pstrcatv(r->pool, iov, k, NULL);
        DDD(dprintf(r->pool, "params: %s", params));
        rv = zdcp_classify(conf->zdcp, apr_pstrcat(r->pool, params, "<url: ><category: >", NULL), &cat);
        if (rv < 0) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                    "zdcp: %s: Failed to classify the data: %s",
                    conf->dir, zdcp_error());
            return HTTP_INTERNAL_SERVER_ERROR;
        }

        DDD(dprintf(r->pool, "category: %s", cat));
        free((void *)cat);
    }
    return OK;
}

static int zdcp_handler(request_rec *r) {
    zdcp_conf_t *conf = ap_get_module_config(r->per_dir_config, &zdcp_module);
    apr_table_t* query_args;

    if (!conf || !conf->enabled)
        return DECLINED;

    query_args = process_args(r);
    //DDD("I'm here! -- agentzh");
    switch (r->method_number) {
        case M_POST:
            return process_post_request(r, conf);
            break;
        default:
            return HTTP_METHOD_NOT_ALLOWED;
            break;
    }
    return DECLINED;
}

static apr_table_t* process_args(request_rec *r) {
    const char *query_string = r->args; /* QUERY_STRING after ? in request */
    apr_table_t *query_args;  /* parsed (hashified) QUERY_STRING */
    char* pair;

    /* Parse the query string into a table called query_args.
     * This is bullet-proof in the event query_string is NULL. */
    query_args = apr_table_make(r->pool, 10);
    if (query_string != NULL) {
        while (*query_string &&
                (pair = ap_getword(r->pool, &query_string, '&'))) {
            const char *key;
            key = ap_getword(r->pool, (const char **)&pair, '=');
            /* pair is now a misnomer, as ap_getword has updated it to point
             * past the =, which would make it just the value */
            apr_table_add(query_args, key, pair);
        }
    }
    return query_args;
}

static int zdcp_post_config(apr_pool_t *p, apr_pool_t *plog,
        apr_pool_t *ptemp, server_rec *s) {
    int threaded_mpm;
    _conf_list_entry_t *d, *n;
    zdcp_conf_t *conf;

    ap_mpm_query(AP_MPMQ_IS_THREADED, &threaded_mpm);
    if (threaded_mpm) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s,
                "Zdcp not supported for threaded Apache. "
                "Consider using the prefork mode instead.");
        return DONE;
    }

    for (d = APR_RING_FIRST(&global_conf_list);
            d != APR_RING_SENTINEL(&global_conf_list, _conf_list_entry, link);
            d = n) {
        n = APR_RING_NEXT(d, link);
        conf = d->conf;
        if (!conf->zdcp) {
            if (conf->enabled == ZDCP_ENABLE_UNSET) {
                conf->enabled = DEFAULT_ZDCP_ENABLE_VALUE;
            }
            if (!conf->enabled) {
                continue;
            }
            if (conf->conf_path == NULL) {
                ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s,
                        "The ZdcpConfigFile config directive is required when ZdcpEnable is set On in Location %s.", conf->dir);
                return DONE;
            }
            conf->zdcp = zdcp_create(conf->conf_path);
            if (conf->zdcp == NULL) {
                ap_log_error(APLOG_MARK, APLOG_CRIT, 0, s,
                        "zdcp_create failed to init with config file %s in Location %s: %s",
                        conf->conf_path, conf->dir, zdcp_error());
                return DONE;
            }
        }
    }

    return OK;
}

static void zdcp_register_hooks(apr_pool_t *p) {
    //fprintf(stderr, "HOOK!!!");
    ap_hook_handler(zdcp_handler, NULL, NULL, APR_HOOK_FIRST);
    ap_hook_post_config(zdcp_post_config, NULL, NULL, APR_HOOK_MIDDLE);
    APR_RING_INIT(&global_conf_list, _conf_list_entry, link);
}

static void* create_dir_config(apr_pool_t *p, char *dir) {
    zdcp_conf_t *conf;
    _conf_list_entry_t *d;

    conf = apr_pcalloc(p, sizeof(zdcp_conf_t));
    conf->enabled = DEFAULT_ZDCP_ENABLE_VALUE;
    conf->dir = apr_pstrdup(p, dir);

    d = apr_pcalloc(p, sizeof(_conf_list_entry_t));
    d->conf = conf;
    APR_RING_INSERT_TAIL(&global_conf_list, d, _conf_list_entry, link);

    return (void*) conf;
}

// XXX this merge callback won't be fired for unknown reasons :((
static void* merge_dir_config(apr_pool_t *p, void *orig, void *new) {
    zdcp_conf_t *conf_orig = (zdcp_conf_t*)orig;
    zdcp_conf_t *conf_new  = (zdcp_conf_t*)new;

    if (conf_new->conf_path == NULL) {
        conf_new->conf_path = conf_orig->conf_path;
    }
    if (conf_new->enabled == ZDCP_ENABLE_UNSET) {
        conf_new->enabled = conf_orig->enabled;
    }
    return (void *)conf_new;
}

static const char *set_zdcp_config_file(cmd_parms *parms, void *dconf,
                                            const char *arg) {
    zdcp_conf_t *conf = (zdcp_conf_t *)dconf;
    conf->conf_path = apr_pstrdup(parms->pool, arg);
    return NULL;
}

static const char *set_zdcp_enable(cmd_parms *parms, void *dconf,
                                            const int arg) {
    zdcp_conf_t *conf = (zdcp_conf_t *)dconf;
    conf->enabled = arg;
    return NULL;
}

static const command_rec zdcp_cmds[] = {
    AP_INIT_TAKE1("ZdcpConfigFile", set_zdcp_config_file, NULL, OR_LIMIT, "Absolute path to the ZDCP's config file (required)"),
    AP_INIT_FLAG("ZdcpEnable", set_zdcp_enable, NULL, OR_LIMIT, "Whether enable ZDCP or not"),
    {NULL}
};

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA zdcp_module = {
    STANDARD20_MODULE_STUFF,
    create_dir_config,      /* create per-dir config struct */
    merge_dir_config,       /* merge per-dir config struct */
    NULL,
    NULL,
    zdcp_cmds,              /* table of config file cmds */
    zdcp_register_hooks     /* register hooks */
};

