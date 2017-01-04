#ifndef WET_SELECTOR_H_
#define WET_SELECTOR_H_

#include "FeatureSelector.h"
#include "math.h"

namespace zdcp {

class WETSelector: public FeatureSelector {
protected:
#if 0
     chi-square
         c   !c
     x   A    B
    !x   C    D

	N = (A+B+C+D)
	P(c) = (A+C)/N
	P(c|x) = A/N
    WE(x) = AVG( log( P(c|x)*(1-P(c))/(P(c)*(1-P(c|x))) )
#endif 

    virtual double score(uint32_t tid, DocOfCategory numDC)
    {
        double                          score = 0.0;
        double                          num;
        double                          per;
        double                          rate;
        DocOfCategory::iterator         it;

        for (it = numDC.begin(); it != numDC.end(); it++) {
            uint32_t cid = it->first;

            if ((per = static_cast<double>(m_docsPerCategory[cid])) == 0.0)
                continue;
            num = static_cast<double>(it->second);

            /* WET */
            // A = num : number of documents with token tid in this category
            // A + C = per :number of documents in this category

            // N: number of documents
            double N = static_cast<double>(m_numDocuments);

			// P(c|x)
			double Pcx = num/N;

			//P(c) = rate
			rate = per / static_cast<double>(m_numDocuments);

            // WET Formula
            double current = log( Pcx * (1 - rate)/(rate * (1 - Pcx)) );
            
            calculateScore(score, current, per, rate, m_calculateType); 
        }
        return score;
    }
};

} //zdcp


#endif // WET_SELECTOR_H_
// vim: ts=4 sw=4 ai cindent et
