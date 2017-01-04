#include <math.h>
#include <cassert>
#include "TFIDFWeightCalculator.h"

namespace zdcp {

void TFIDFWeightCalculator::calculate(DocumentList &docList,
     const Feature &feature)
{
    std::vector<Document>::iterator               it;
    bool                                          useFeature;
    FILE                                          *fp;
    int                                           numField;
    uint32_t                                      cid, numDoc;

    if (!(fp = fopen(m_dpcIndexFile.c_str(), "r"))) 
        THROW_RUNTIME_ERROR(std::string("can not read " + m_dpcIndexFile));

    m_trainingDocSize = 0;
    while ((numField = fscanf(fp, "%d %d\n", &cid, &numDoc)) != EOF) {
        if (numField != 2)
            THROW_RUNTIME_ERROR("invalid dpc format");
        m_docPerCategory[cid] = numDoc;
        m_trainingDocSize += numDoc;
    }

    fclose(fp);

    useFeature = (feature.size())?true:false;
    for (it = docList.Documents.begin(); it < docList.Documents.end(); it++) {
        std::vector<uint32_t>::iterator       termIt;
        std::vector<std::string>::iterator    fieldIt;
        uint32_t                              cid;

        cid = it->Category;

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
                    it->Feature[tid] += (m_fieldWeight[*fieldIt] / numTerms)
                                      * log(m_trainingDocSize / m_docPerCategory[cid]);
                }

            }
        }

    }

}


} //zdcp


// vim: ts=4 sw=4 et ai cindent
