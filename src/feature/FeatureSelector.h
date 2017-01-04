#ifndef FEATURE_SELECTOR_H_
#define FEATURE_SELECTOR_H_

#include <stdint.h>

#include <vector>
#include <map>
#include <stdexcept>
#include <iostream>
#include <string>

#include "trie/Trie.h"
#include "config/SimpleConfig.h"
#include "document/DocumentList.h"

namespace zdcp {

enum CST
{
    CST_SUM = 0,
    CST_AVG,
    CST_MAX
};

typedef std::pair<uint32_t, double> FeatureScore;
typedef __gnu_cxx::hash_map<uint32_t, uint32_t> DocOfCategory;
typedef __gnu_cxx::hash_map<uint32_t, DocOfCategory > DocOfTerm;
typedef __gnu_cxx::hash_map<uint32_t, double > Feature;
typedef std::vector<FeatureScore > FeatureQueue;

class FeatureSelector {
protected:

    // DocOfTerm: Given termid, the number of documents which contain the given
    // term in a given category can be found with m_numDocTerm;
    DocOfTerm                         m_numDocTerm;

    // number of documents per category, indiced by category id;
    std::map<uint32_t, uint32_t>      m_docsPerCategory;
    
    uint32_t                          m_numDocuments;
    int                               m_calculateType;
    uint32_t                          m_maxFeatures;
    std::vector<std::string>          m_documentFields;
    FeatureQueue                      m_featureQ;
    std::string                       m_featureIndexFile;
    std::string                       m_dpcIndexFile;
    std::map<std::string, double>     m_fieldWeight;
    std::string                       m_featureDumpFile;

    void calculateDocuments(DocumentList &docList);
    void calculateFeatureScore();

    virtual double score(uint32_t tid, DocOfCategory numDC)
    {
        double                          score;
        double                          num;
        double                          per;
        double                          rate;
        DocOfCategory::iterator         it;

        score = 0.0;
        for (it = numDC.begin(); it != numDC.end(); it++) {
            uint32_t cid = it->first;

            num = static_cast<double>(it->second);
            per = static_cast<double>(m_docsPerCategory[cid]);
            rate = per / static_cast<double>(m_numDocuments);
            
            calculateScore(score, num, per, rate, m_calculateType); 
        }

        return score;
    }

    void calculateScore(double& score, const double num,
         const double per, const double rate, const int type)
    {
        switch (type) {
            case CST_SUM:
                score += num;
                break;
            case CST_AVG:
                score += rate * num;
                break;
            case CST_MAX:
                if (num > score) score = num;
                break;
        }
    }

    void saveFeature(DocumentList &docList, const Feature &feature);

public:
    FeatureSelector()
        :m_calculateType(CST_AVG),
         m_featureIndexFile("feature.idx"),
         m_dpcIndexFile("")
    {
    }
    virtual void configure(SimpleConfig &config)
    {
        int i = -1;

        config.getValue("feature.dump", m_featureDumpFile);
        config.getValue("feature.number", i);
        config.getValue("index.feature", m_featureIndexFile);
        config.getValue("index.dpc", m_dpcIndexFile);
        config.getValue("document.fields", m_documentFields);
        m_maxFeatures = (uint32_t)i;
        std::cerr << "Max No. Features: " << m_maxFeatures << std::endl;

        // load weight
        config.getValue("document.fields", m_documentFields);

        std::vector<std::string>::iterator    fieldIt;

        for (fieldIt = m_documentFields.begin();
             fieldIt != m_documentFields.end();
             fieldIt++)
        {
        	std::string  prefix = "weight.";
            config.getValue(prefix.append(*fieldIt), m_fieldWeight[*fieldIt]);
            if (m_fieldWeight[*fieldIt] < 0.001) {
                std::cerr << "warning weight of field " << *fieldIt << " is too small" << std::endl;
            }
        }
    }

    void select(DocumentList &docList, Feature &feature);
    virtual ~FeatureSelector()
    {
    }
};

} // zdcp

#endif // FEATURE_SELECTOR_H_

// vim: ts=4 sw=4 ai cindent et
