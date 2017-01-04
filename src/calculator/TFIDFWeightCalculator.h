#ifndef TFIDF_WEIGHT_CALCULATOR_H_
#define TFIDF_WEIGHT_CALCULATOR_H_

#include <stdint.h>
#include <map>
#include <ext/hash_map>

#include "WeightCalculator.h"

namespace zdcp {

class TFIDFWeightCalculator :public WeightCalculator {
protected:
    std::map<uint32_t, uint32_t> m_docPerCategory;
    std::string                  m_dpcIndexFile;
    uint32_t                     m_trainingDocSize;
public:
	TFIDFWeightCalculator(): WeightCalculator()
	{
	}
	void calculate(DocumentList &docList, const Feature &feature);
    void configure(SimpleConfig &config)
    {
        WeightCalculator::configure(config);
        config.getValue("index.dpc", m_dpcIndexFile);

    }
};

} // zdcp

#endif //TFIDF_WEIGHT_CALCULATOR_H_

// vim: ts=4 sw=4 et ai cindent
