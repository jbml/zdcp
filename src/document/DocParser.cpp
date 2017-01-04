#include <math.h>
#include "DocParser.h"

#include "trie/Trie.h"

namespace zdcp {

DocParser::DocParser()
    :m_chineseParser(NULL),
     m_categoryFieldName("category"),
     m_termIndexFile("term.idx"),
     m_featureIndexFile("feature.idx"),
     m_categoryIndexFile("category.idx"),
     m_onlyFeature(0),
     m_stopWordIndexFile("stopword.idx"),
     m_stopRegexIndexFile("stopword.regex"),
     m_stopWordTrie(NULL)

{
    bamboo::ParserFactory               *factory;

    factory = bamboo::ParserFactory::get_instance();
    m_chineseParser = factory->create("crf_seg"); // Using CRF Segmentation
}

void DocParser::configure(SimpleConfig &config)
{
    config.getValue("index.term", m_termIndexFile);
    config.getValue("index.category", m_categoryIndexFile);
    config.getValue("index.feature", m_featureIndexFile);
    config.getValue("index.stopword", m_stopWordIndexFile);
    config.getValue("index.stopregex", m_stopRegexIndexFile);
    config.getValue("document.fields", m_documentFields);
    config.getValue("predictor.onlyfeature", m_onlyFeature);
}

void DocParser::convertFinish(DocumentList &docList, ParseMode mode)
{
    std::cerr << docList.Documents.size() << "." << std::endl;
    if (mode != PM_LOAD)
        saveIndex(docList);
}

void DocParser::convert(DocumentList &docList, RawDocument *raw, ParseMode mode)
{
    Document                                doc;
    Trie                                    *termTrie = NULL;
    Trie                                    *categoryTrie = NULL;
    std::vector<std::string>::iterator      fieldIt;

    std::map<std::string, std::vector<bamboo::Token *> > tokenizedTerms;

    loadStopWordTrie();
    loadStopWordRegex();

    // make unique category list
    if (docList.CategoryIndex.find(raw->Category.c_str()) 
        == docList.CategoryIndex.end()) 
    {
        docList.CategoryIndex[raw->Category.c_str()] = docList.CategoryIndex.size();
    }
    doc.Category = docList.CategoryIndex[raw->Category.c_str()];

    for (fieldIt = m_documentFields.begin();
         fieldIt != m_documentFields.end();
         fieldIt++)
    {
        m_chineseParser->setopt(BAMBOO_OPTION_TEXT, raw->Fields[*fieldIt].c_str());
        m_chineseParser->parse(tokenizedTerms[*fieldIt]);
        doc.TermIds[*fieldIt].reserve(tokenizedTerms[*fieldIt].size());
    }

    std::vector<bamboo::Token *>::iterator it;
    
    if (mode == PM_LOAD) {
        termTrie = new Trie((m_onlyFeature)?
                    m_featureIndexFile.c_str():m_termIndexFile.c_str());
        categoryTrie = new Trie(m_categoryIndexFile.c_str());
    }

    // convert tokenized terms to termid
    for (fieldIt = m_documentFields.begin();
         fieldIt != m_documentFields.end();
         fieldIt++)
    {
        for (it = tokenizedTerms[*fieldIt].begin();
             it < tokenizedTerms[*fieldIt].end(); 
             it++) 
        {
            const char *token = (*it)->get_orig_token();

            if (mode == PM_LOAD) {
                int tid; 

                if ((tid = termTrie->search(token)))
                    doc.TermIds[*fieldIt].push_back(tid);
            } else {

                if (!m_stopWordTrie->search(token) && !matchStopWordRegex(token)) {
                    // make unique term list
                    if (docList.TermIndex.find(token) == docList.TermIndex.end()) {
                        docList.TermIndex[token] = docList.TermIndex.size();
                    }

                    doc.TermIds[*fieldIt].push_back(docList.TermIndex[token]);
                }

            } 

            delete *it;
        }
    }

    docList.Documents.push_back(doc);
    
    if (mode == PM_LOAD) {
        delete termTrie;
        delete categoryTrie;
    }

    if ((docList.Documents.size() + 1) % 101 == 0)
        std::cerr << docList.Documents.size() << "..";
}

void DocParser::saveIndex(DocumentList &docList)
{
    std::cerr << "Storing terms index  to " << m_termIndexFile << std::endl;

    Trie                    termTrie;
    Trie                    categoryTrie;
    HashIndex::iterator     termIt;

    for (termIt = docList.TermIndex.begin();
         termIt != docList.TermIndex.end();
         termIt++)
    {
        termTrie.insert(termIt->first.c_str(), termIt->second);
    }

    termTrie.save(m_termIndexFile.c_str());

    std::cerr << "Storing category index to " << m_categoryIndexFile << std::endl;

    for (termIt = docList.CategoryIndex.begin();
         termIt != docList.CategoryIndex.end();
         termIt++)
    {
        categoryTrie.insert(termIt->first.c_str(), termIt->second);
    }

    categoryTrie.save(m_categoryIndexFile.c_str());
}

} //zdcp

// vim: ts=4 sw=4 ai cindent et
