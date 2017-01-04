#ifndef TW_WEIGHT_CALCULATOR_H_
#define TW_WEIGHT_CALCULATOR_H_

#include <stdint.h>
#include <ext/hash_map>

#include "WeightCalculator.h"

namespace zdcp {

class TWWeightCalculator :public WeightCalculator {
public:
	TWWeightCalculator(): WeightCalculator()
	{
	}
	void calculate(DocumentList &docList, const Feature &feature);
};

} // zdcp

#endif //TW_WEIGHT_CALCULATOR_H_

// vim: ts=4 sw=4 et ai cindent
