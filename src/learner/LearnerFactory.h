#ifndef LEARNER_FACTORY_H_
#define LEARNER_FACTORY_H_

#include "Learner.h"
#include "SvmLearner.h"
#include "LogLinearLearner.h"

#include "common/Factory.h"

#define REGISTER_LEARNER(N) if ( strcmp( name, #N ) == 0 ) \
        return new N##Learner();


namespace zdcp {

class LearnerFactory: public Factory<LearnerFactory, Learner> {

protected:    
    Learner *createProduct(const char *name)
    {
        REGISTER_LEARNER(Svm);
        REGISTER_LEARNER(LogLinear);
        THROW_RUNTIME_ERROR(std::string("unknow ") + getFactoryName()
                + " name : " + std::string(name));
    }

public:
    std::string getFactoryName()
    {
        return "classify learner";
    }

};

} //zdcp

#undef REGISTER_LEARNER

#endif // LEARNER_FACTORY_H_
// vim: ts=4 sw=4 ai cindent et
