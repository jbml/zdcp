#include <algorithm>
#include <cassert>

#include "FeatureSelector.h"

namespace zdcp {

bool score_compare(const FeatureScore &lhs, const FeatureScore &rhs)
{
    return lhs.second > rhs.second;
}

void FeatureSelector::calculateDocuments(DocumentList &docList)
{
    uint32_t                                      cnt;
    std::vector<Document>::iterator               it;

    m_numDocuments = docList.Documents.size();

    cnt = 1;
    std::cerr << "Calculating documents ";

    for (it = docList.Documents.begin(); it < docList.Documents.end(); it++) {
        std::vector<uint32_t>::iterator       termIt;
        uint32_t                              cid;
        std::vector<std::string>::iterator    fieldIt;
        
        cid = it->Category;
        assert(cid > 0);

        m_docsPerCategory[cid]++; // calculate documents per category

        for (fieldIt = m_documentFields.begin();
             fieldIt != m_documentFields.end();
             fieldIt++)
        {
            if (m_fieldWeight[*fieldIt] < 0.0001)
                continue;

            for (termIt = it->TermIds[*fieldIt].begin();
                 termIt != it->TermIds[*fieldIt].end();
                 termIt++)
            {
                uint32_t tid = *termIt;

                assert(tid > 0);

                m_numDocTerm[tid][cid]++;
            }
        }

        if (cnt % 100 == 0) 
            std::cerr << cnt << "..";
        cnt++;
    }
    std::cerr << cnt - 1 << "." << std::endl;
}

void FeatureSelector::calculateFeatureScore()
{
    DocOfTerm::iterator     it;

    m_featureQ.reserve(m_numDocTerm.size());
    for (it = m_numDocTerm.begin(); it != m_numDocTerm.end(); it++) {
        uint32_t tid = it->first;

        m_featureQ.push_back(std::make_pair(tid, score(tid, it->second)));
    }
}

void
FeatureSelector::select(DocumentList &docList, Feature &feature)
{
    ofstream dumpfs;

    std::cerr << "No. documents: " << docList.Documents.size() << std::endl;
    std::cerr << "No. categories: " << docList.CategoryIndex.size() 
              << std::endl;
    std::cerr << "No. terms: " << docList.TermIndex.size() << std::endl;

    calculateDocuments(docList);
    calculateFeatureScore();
    std::sort(m_featureQ.begin(), m_featureQ.end(), score_compare);
    
    if (m_maxFeatures < m_featureQ.size())
        m_featureQ.erase(m_featureQ.begin() + m_maxFeatures, m_featureQ.end());

    std::cerr << "No. features: " << m_featureQ.size() << std::endl;    

    if (!m_featureDumpFile.empty()) {
        dumpfs.open(m_featureDumpFile.c_str());
        if (!dumpfs.is_open()) {
            std::cerr << "warning store features into " 
                      << m_featureDumpFile << std::endl;
        }
    }


	// convert features
	std::cerr << "Converting features ..." << std::endl;
    for (uint32_t i = 0; i < m_featureQ.size(); i++)
        feature[m_featureQ[i].first] = 0.0;//m_featureQ[i].second;

    // save features and adjust the number of features and terms
    std::cerr << "Storing feature index to " << m_featureIndexFile << std::endl;

    Trie                    featureTrie;
    HashIndex::iterator     termIt;
    bool                    dump = dumpfs.is_open();
    for (termIt = docList.TermIndex.begin();
         termIt != docList.TermIndex.end();
         termIt++)
    {
        Feature::const_iterator featureIt = feature.find(termIt->second);
        if (featureIt != feature.end()) {
            featureTrie.insert(termIt->first.c_str(), featureIt->first);
            if (dump)
                dumpfs << termIt->first.c_str() << "\t" << featureIt->second << std::endl;
		}
    }

    if (dump)
        dumpfs.close();

    featureTrie.save(m_featureIndexFile.c_str());

    // save idf
    if (!m_dpcIndexFile.empty()) {
        std::cerr << "Storing IDF index to " << m_dpcIndexFile << std::endl;

        std::map<uint32_t, uint32_t>::iterator dpIt;
        FILE                                   *fp;

        if (!(fp = fopen(m_dpcIndexFile.c_str(), "w+")))
            THROW_RUNTIME_ERROR(std::string("can not write to file") 
                                     + m_dpcIndexFile);

        for (dpIt = m_docsPerCategory.begin();
             dpIt != m_docsPerCategory.end(); 
             dpIt++)
        {
            fprintf(fp, "%d %d\n", dpIt->first, dpIt->second); 
        }
        
        fclose(fp);
    }
    
}


} // zdcp

// vim: ts=4 sw=4 ai cindent et
