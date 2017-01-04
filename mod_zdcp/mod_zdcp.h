#ifndef MOD_ZDCP_H
#define MOD_ZDCP_H

typedef struct zdcp_conf {
    char *dir;  /* location dir */
    char *conf_path;
    int enabled;
    void *zdcp;
} zdcp_conf_t;

#endif

