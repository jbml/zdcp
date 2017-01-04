#ifndef FEATURE_SELECTOR_FACTORY_H_
#define FEATURE_SELECTOR_FACTORY_H_

#include "FeatureSelector.h"
#include "ChiSquareSelector.h"
#include "MISelector.h"
#include "WETSelector.h"

#include "common/Factory.h"

#define REGISTER_SELECTOR(N) if ( strcmp( name, #N ) == 0 ) \
        return new N##Selector();

namespace zdcp {

class FeatureSelectorFactory: 
    public Factory<FeatureSelectorFactory, FeatureSelector> {

protected:    
    FeatureSelector *createProduct(const char *name)
    {
        REGISTER_SELECTOR(ChiSquare);
        REGISTER_SELECTOR(MI);
		REGISTER_SELECTOR(WET);
        THROW_RUNTIME_ERROR(std::string("unknow ") + getFactoryName()
                + " name : " + std::string(name));
    }

public:
    std::string getFactoryName()
    {
        return "feature selector";
    }

};

} //zdcp

#undef REGISTER_SELECTOR

#endif // FEATURE_SELECTOR_FACTORY_H_
// vim: ts=4 sw=4 ai cindent et
