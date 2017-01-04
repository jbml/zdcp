#ifndef CHI_SQUARE_SELECTOR_H_
#define CHI_SQUARE_SELECTOR_H_

#include "FeatureSelector.h"

namespace zdcp {

class ChiSquareSelector: public FeatureSelector {
protected:
#if 0
     chi-square
         c   !c
     x   A    B
    !x   C    D
    CHI(x,c) = (N * (AD-CB)^2) / ( (A+C)*(B+D)*(A+B)*(C+D) )
#endif 

    virtual double score(uint32_t tid, DocOfCategory numDC)
    {
        double                          score = 0.0;
        double                          sum = 0.0;
        double                          num;
        double                          per;
        double                          rate;
        DocOfCategory::iterator         it;

        for (it = numDC.begin(); it != numDC.end(); it++)
            sum += static_cast<double>(it->second);

        for (it = numDC.begin(); it != numDC.end(); it++) {
            uint32_t cid = it->first;

            if ((per = static_cast<double>(m_docsPerCategory[cid])) == 0.0)
                continue;
            rate = per / static_cast<double>(m_numDocuments);
            num = static_cast<double>(it->second);

            /* CHI Square */
            // A: number of documents with token tid in this category
            double A = num; 
            // B: number of documents without token tid in this category
            double B = sum - num;
            // C: number of documents with token tid  not in this category
            double C = per - num;
            // D: number of documents without token tid not in this category
            double D = m_numDocuments - A - B - C;

            double AD_CB = A * D - C * B; // a replacement

            // ChiSquare Formula
            double current = (m_numDocuments * AD_CB * AD_CB) / 
                             ((A + C) * (B + D) * (A + B) * (C + D));
            
            calculateScore(score, current, per, rate, m_calculateType); 
        }
        return score;
    }
};

} //zdcp


#endif // CHI_SQUARE_SELECTOR_H_
// vim: ts=4 sw=4 ai cindent et
