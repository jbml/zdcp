#ifndef WEIGHT_CALCULATOR_FACTORY_H_
#define WEIGHT_CALCULATOR_FACTORY_H_

#include <stdexcept>

#include "WeightCalculator.h"
#include "TWWeightCalculator.h"
#include "TFWeightCalculator.h"
#include "TFIDFWeightCalculator.h"

#include <common/Factory.h>

#define REGISTER_CALCULATOR(N) if ( strcmp( name, #N ) == 0 ) \
        return new N##WeightCalculator();

namespace zdcp {

class WeightCalculatorFactory:
    public Factory<WeightCalculatorFactory, WeightCalculator> {

protected:    
    WeightCalculator *createProduct(const char *name)
    {
        REGISTER_CALCULATOR(TW);
        REGISTER_CALCULATOR(TF);
        REGISTER_CALCULATOR(TFIDF);
        THROW_RUNTIME_ERROR(std::string("unknow ") + getFactoryName()
                + " name : " + std::string(name));
    }

public:
    std::string getFactoryName()
    {
        return "weight calculator";
    }

};

} // zdcp

#undef REGISER_CALCULATOR                   

#endif // WEIGHT_CALCULATOR_FACTORY_H_

// vim: ts=4 sw=4 ai cindent et
