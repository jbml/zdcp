#include <cassert>
#include "TFWeightCalculator.h"

namespace zdcp {

void TFWeightCalculator::calculate(DocumentList &docList,
     const Feature &feature)
{
    std::vector<Document>::iterator               it;
    bool                                          useFeature;

    useFeature = (feature.size())?true:false;
    for (it = docList.Documents.begin(); it < docList.Documents.end(); it++) {
        std::vector<uint32_t>::iterator       termIt;
        std::vector<std::string>::iterator    fieldIt;

        for (fieldIt = m_documentFields.begin();
             fieldIt != m_documentFields.end();
             fieldIt++)
        {
            double numTerms = static_cast<double>(it->TermIds[*fieldIt].size());

            for (termIt = it->TermIds[*fieldIt].begin();
                 termIt != it->TermIds[*fieldIt].end();
                 termIt++)
            {
                uint32_t tid = *termIt;

                assert(tid > 0);

                if (!useFeature || (feature.find(tid) != feature.end())) {
                    it->Feature[tid] += m_fieldWeight[*fieldIt] / numTerms;
                }

            }
        }

    }

}


} //zdcp


// vim: ts=4 sw=4 et ai cindent
