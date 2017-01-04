#ifndef ZDCP_H_
#define ZDCP_H_

#ifdef __cplusplus
extern "C" {
#endif
const char *zdcp_error(void);
void *zdcp_create(const char *filename);
void zdcp_destroy(void *handle);
const char *zdcp_getclassname(void *handle, unsigned int id);
int zdcp_classify(void *handle, const char *document, const char **classname);
int zdcp_classify_with_prob(void *handle, const char *document, const double **probs);

#ifdef __cplusplus
}
#endif

#endif // ZDCP_H_
// vim: ts=4 sw=4 ai cindent et
