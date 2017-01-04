#ifndef LEARNER_H_
#define LEARNER_H_

#include "config/SimpleConfig.h"
#include "document/DocumentList.h"

namespace zdcp {

class OnPredict {
public:    
    virtual bool operator() (uint32_t categoryId, double *probabilities, uint32_t numClass) = 0;
    virtual ~OnPredict()
    {
    }
};

class Learner {
protected:
    int         m_showVector;
public:
   virtual void configure(SimpleConfig &config)
   {
        config.getValue("generic.showvector", m_showVector);
   }
   virtual void learn(DocumentList &docList) = 0;
   virtual void predict(DocumentList &docList, OnPredict &onPredict) = 0;
   virtual ~Learner()
   {
   }
};

}// zdcp;

#endif // LEARNER_H_

// vim: ts=4 sw=4 et ai cindent
