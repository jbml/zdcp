#ifndef DOC_PARSER_H_
#define DOC_PARSER_H_

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "pcre.h"
#include "bamboo/bamboo.hxx"

#include "DocumentList.h"
#include "trie/Trie.h"
#include "config/SimpleConfig.h"


/* Blame jianingy for such a ridiculous parser pattern */

namespace zdcp {

enum ParseMode
{
    PM_UNKNOW = 0,
    PM_LOAD
};

struct RawDocument {
    std::map<std::string, std::string>  Fields;
    std::string                         Category;
};

class DocParser {
protected:
    bamboo::Parser             *m_chineseParser;
    std::map<std::string, int>  m_catIndex;

    std::vector<std::string>    m_documentFields;
    std::string                 m_categoryFieldName;
    std::string                 m_termIndexFile;
    std::string                 m_featureIndexFile;
    std::string                 m_categoryIndexFile;
    int                         m_onlyFeature;
    std::string                 m_stopWordIndexFile;
    std::string                 m_stopRegexIndexFile;
    Trie                        *m_stopWordTrie;

    std::vector<std::pair<pcre *, pcre_extra *> >  m_stopWordRegex;
    
    bool isStopWord(const char *s);

    void loadStopWordTrie() 
    {
        if (!m_stopWordTrie)
            m_stopWordTrie = new Trie(m_stopWordIndexFile.c_str());
    }

    void loadStopWordRegex()
    {
        const char      *error;
        int             errorOffset;

        if (m_stopWordRegex.size() == 0) {
            std::ifstream ifs(m_stopRegexIndexFile.c_str());
            std::string   s;

            if (ifs.is_open()) {
                while(!(std::getline(ifs, s).eof())) {
                    pcre             *re = NULL;
                    pcre_extra       *pe = NULL;
                    
                    re = pcre_compile(s.c_str(), PCRE_UTF8, &error, 
                                      &errorOffset, NULL);
                    if (!re)
                        THROW_RUNTIME_ERROR(error + errorOffset);
                    
                    pe = pcre_study(re, 0, &error);

                    m_stopWordRegex.push_back(std::make_pair(re, pe));
                }    
            } else {
                THROW_RUNTIME_ERROR(std::string("can not open stopword regex ") + m_stopRegexIndexFile);
            }
        }
    }

    bool matchStopWordRegex(const char *s)
    {
        const int VECTOR_SIZE = 30;
            
        std::vector<std::pair<pcre *, pcre_extra *> >::iterator it;
        int rc;
        int ovector[VECTOR_SIZE];

        for (it = m_stopWordRegex.begin(); it < m_stopWordRegex.end(); it++) {
            rc = pcre_exec(it->first, it->second, s,
                           (int)strlen(s), 0, 0, ovector, VECTOR_SIZE); 
            if (rc >= 0) return true;
        }

        return false;
    }
    
public:
    DocParser();
    virtual void parseText(const char *text, DocumentList &docList, ParseMode mode)=0;
    virtual void parseFile(const char *filename, DocumentList &docList, ParseMode mode)=0;
    virtual void configure(SimpleConfig &config);
    virtual void convert(DocumentList &docList, RawDocument *raw, ParseMode mode);
    virtual void convertFinish(DocumentList &docList, ParseMode mode);
	void saveIndex(DocumentList &docList);

    virtual ~DocParser()
    {

        std::vector<std::pair<pcre *, pcre_extra *> >::iterator it;

        for (it = m_stopWordRegex.begin(); it != m_stopWordRegex.end(); it++) {
            free(it->first);
            free(it->second);
        }

        if (m_chineseParser)
            delete m_chineseParser;
        if (m_stopWordTrie)
            delete m_stopWordTrie;
    }
};

} // zdcp

#endif // DOC_PARSER_H_

// vim: ts=4 sw=4 ai cindent et
