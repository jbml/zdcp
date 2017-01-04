#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <limits.h>
#include "DcpParser.h"

namespace zdcp {
void DcpParser::parseText(const char *str, DocumentList &docList, ParseMode mode)
{
#define __X(x) #x
#define _X(x) __X(x)
    uint32_t                            cnt = 1;
    std::vector<std::string>            fields = m_documentFields;
    std::vector<std::string>::iterator  it;
    RawDocument                         raw;
    int                                 numField;
    char                                *text;
    char                                fmt[LINE_MAX] = "";
    bool                                flag = true;


    std::cerr << "Parsing Document ";
    fields.push_back(m_categoryFieldName);

    text = new char[BUFFER_MAX + 1];
    *text = '\0';

    while (flag) {
        for (it = fields.begin(); it != fields.end(); it++) {

            if (str == NULL) {
                flag = false;
                break;
            }

            snprintf(fmt, LINE_MAX, "<%s:%%%d[^>]> ", it->c_str(), BUFFER_MAX);
            numField = sscanf(str, fmt, text);

            if (numField == EOF) {
                flag = false;
                break;
            } else if (numField != 1 && numField != EOF) {
                snprintf(text, LINE_MAX,
                         "missing field '%s' at line %u", it->c_str(), cnt);
                THROW_RUNTIME_ERROR(text);
            }

            str = strchr(str + 1, '<');
            if (*it == m_categoryFieldName) 
                raw.Category.assign(text);
            else
                raw.Fields[*it].assign(text);


        }

        if (flag) {
            convert(docList, &raw, mode);
            ++cnt;
        }

    }

    convertFinish(docList, mode);

    delete []text;
#undef _X
#undef __X
}

void DcpParser::parseFile(const char *filename, DocumentList &docList, ParseMode mode)
{
#define __X(x) #x
#define _X(x) __X(x)
    FILE            *fp = NULL;
    
    if (!(fp = fopen(filename, "r")))
        THROW_RUNTIME_ERROR(std::string("can not open document ") + filename);


    uint32_t                            cnt = 1;
    std::vector<std::string>            fields = m_documentFields;
    std::vector<std::string>::iterator  it;
    RawDocument                         raw;
    int                                 numField;
    char                                *text;
    char                                fmt[LINE_MAX] = "";
    bool                                flag = true;


    std::cerr << "Parsing Document ";
    fields.push_back(m_categoryFieldName);

    text = new char[BUFFER_MAX + 1];
    *text = '\0';

    while (flag) {
        for (it = fields.begin(); it != fields.end(); it++) {

            snprintf(fmt, LINE_MAX, "<%s:%%%d[^>]> ", it->c_str(), BUFFER_MAX);
            numField = fscanf(fp, fmt, text);

            if (numField == EOF) {
                flag = false;
                break;
            } else if (numField != 1) {
                snprintf(text, LINE_MAX,
                         "missing field '%s' at line %u", it->c_str(), cnt);
                THROW_RUNTIME_ERROR(text);
            }

//            std::cout << *it << ": " << text << std::endl;
            if (*it == m_categoryFieldName) 
                raw.Category.assign(text);
            else
                raw.Fields[*it].assign(text);

        }

        if (flag) {
            convert(docList, &raw, mode);
            ++cnt;
        }

    }

    convertFinish(docList, mode);

    delete []text;

    fclose(fp);

#undef _X
#undef __X
}

} // zdcp

// vim: ts=4 sw=4 ai cindent et
