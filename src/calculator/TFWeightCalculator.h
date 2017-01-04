#ifndef TF_WEIGHT_CALCULATOR_H_
#define TF_WEIGHT_CALCULATOR_H_

#include <stdint.h>
#include <ext/hash_map>

#include "WeightCalculator.h"

namespace zdcp {

class TFWeightCalculator :public WeightCalculator {
public:
	TFWeightCalculator(): WeightCalculator()
	{
	}
	void calculate(DocumentList &docList, const Feature &feature);
};

} // zdcp

#endif //TF_WEIGHT_CALCULATOR_H_

// vim: ts=4 sw=4 et ai cindent
