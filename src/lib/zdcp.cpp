#include <limits.h>
#include "zdcp.h"
#include "classifier/Classifier.h"

#define CAST_HANDLE zdcp::Classifier *classifier = static_cast<zdcp::Classifier *>(handle);

static char error_buffer[LINE_MAX];

const char *zdcp_error(void)
{
    return error_buffer;
}

void *zdcp_create(const char *filename)
{
    try {
        return new zdcp::Classifier(filename);
    } catch(std::exception &e) {
        snprintf(error_buffer, LINE_MAX, "%s", e.what());
        return NULL;
    }
}

void zdcp_destroy(void *handle)
{
   if (handle == NULL)
       return;
    delete static_cast<zdcp::Classifier *>(handle);
}

const char *zdcp_getclassname(void *handle, unsigned int id)
{
   try { 
       if (handle == NULL)
           return NULL;

       CAST_HANDLE;

       return classifier->getClassNameById(id);
    } catch(std::exception &e) {
        snprintf(error_buffer, LINE_MAX, "%s", e.what());
        return NULL;
    }
}

int zdcp_classify(void *handle, const char *document, const char **classname)
{
   try { 
       if (handle == NULL)
           return -1;

       CAST_HANDLE;

       return classifier->classify(document, classname);
    } catch(std::exception &e) {
        snprintf(error_buffer, LINE_MAX, "%s", e.what());
        return -1;
    }
}

int zdcp_classify_with_prob(void *handle, const char *document, const double **probs)
{
   try { 
       if (handle == NULL)
           return -1;

       CAST_HANDLE;

       return classifier->classify(document, probs);
    } catch(std::exception &e) {
        snprintf(error_buffer, LINE_MAX, "%s", e.what());
        return -1;
    }
}

// vim: ts=4 sw=4 ai cindent et
