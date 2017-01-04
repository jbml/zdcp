#ifndef MI_SELECTOR_H_
#define MI_SELECTOR_H_

#include "math.h"
#include "FeatureSelector.h"

#define MI_OLD_METHOD

namespace zdcp {

class MISelector: public FeatureSelector {
protected:

#if 0
                     P(x, y)
    MI(x,y) = log -------------
                   P(x) * P(y)

    MI(x,y) = H(x) + H(y) - H(x,y)

    * H(x) is the entropy of x
    * MI(x,y) << 0 means that there are no connection
      between x and y

#endif         

    virtual double score(uint32_t tid, DocOfCategory numDC)
    {
        double                          score = 0.0 ;
        double                          sum = 0.0;
        double                          num;
        double                          per;
        double                          rate;
        DocOfCategory::iterator         it;


        for (it = numDC.begin(); it != numDC.end(); it++)
            sum += static_cast<double>(it->second);
#ifdef MI_OLD_METHOD
        // px: probability of documents in this category
        double px = sum / static_cast<double>(m_numDocuments);
        // py: probability of documents not in this category
        double py = 1.0 - px;
#endif
        for (it = numDC.begin(); it != numDC.end(); it++) {
            uint32_t cid = it->first;

            if ((per = static_cast<double>(m_docsPerCategory[cid])) == 0.0)
                continue;

            rate = per / static_cast<double>(m_numDocuments);
            num = static_cast<double>(it->second);
#ifdef MI_OLD_METHOD

            // pcx: probability of documents with token tid in this category
            double pcx = num / static_cast<double>(m_numDocuments);
            // pcy: probability of documents without token tid in this category
            double pcy = (per - num) / static_cast<double>(m_numDocuments);

            // MI Formula:
            // * Why mulitply (rate * px)?
            // * Why no minus H(x,y) ?
            double current = 
                 ((px && pcx)?pcx * log( pcx / (rate * px) ):0.0)
                +((py && pcy)?pcy * log( pcy / (rate * py) ):0.0);

#else            
            // A: number of documents with token tid in this category
            double A = num; 
            // B: number of documents without token tid in this category
            double B = sum - num;
            // C: number of documents with token tid  not in this category
            double C = per - num;
            // D: number of documents without token tid not in this category
            // D: Unused
            // double D = m_numDocuments - A - B - C;

            // UTF-8: 本计算方法来自<<统计自然语言处理>> - 清华大学出版社
            // Page: 345
            double current = log((A * static_cast<double>(m_numDocuments))
                            / ((A + C) * (A + B)));
#endif



            calculateScore(score, current, per, rate, m_calculateType); 
        }
        return score;
    }
};

} //zdcp
#endif // MI_SELECTOR_H

// vim: ts=4 sw=4 ai cindent et
