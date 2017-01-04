#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#include "config/SimpleConfig.h"
#include "common/hash.h"
#include "document/DocParserFactory.h"
#include "feature/FeatureSelectorFactory.h"
#include "calculator/WeightCalculatorFactory.h"
#include "learner/LearnerFactory.h"

#include <iostream>
#include <fstream>

namespace zdcp {

class ClassifierPredict:public OnPredict {

private:
    double                                     m_threshold;
    int                                        m_classId;
    double                                     *m_probs;
    uint32_t                                   m_numClass;
public:
    ClassifierPredict()
        :m_threshold(0.5), m_probs(NULL), m_numClass(0)
    {
    }

    ~ClassifierPredict()
    {
        if (m_probs)
            delete [] m_probs;
    }

    void configure(SimpleConfig &config)
    {
        config.getValue("predictor.threshold", m_threshold);
    }

    int getClassId()
    {
        return m_classId;
    }

    const double *getProbs()
    {
        return m_probs;
    }

    const uint32_t getNumClass()
    {
        return m_numClass;
    }

    bool operator() (uint32_t cid, double *prob, uint32_t numClass)
    {
        if (m_probs == NULL) 
            m_probs = new double[numClass + 1];

        if (m_numClass == 0)
            m_numClass = numClass;

        if (cid == 0 || prob[cid - 1] <= m_threshold) {
            m_classId = 0;
        } else {
            m_classId = cid;
        }

        uint32_t i;

        for (i = 0; i < numClass; i++)
            m_probs[i + 1] = (cid > 0)?prob[i]:0;

        m_probs[0] = m_classId;

        return true;
    }
};


class Classifier {

protected:
    FeatureSelectorFactory              *m_fsFactory;
    DocParserFactory                    *m_dpFactory;
    WeightCalculatorFactory             *m_wcFactory;
    LearnerFactory                      *m_lrFactory;
    SimpleConfig                        m_config; 
    DocParser                           *m_parser;
    FeatureSelector                     *m_selector;
    Learner                             *m_learner;
    WeightCalculator                    *m_calculator;
    ClassifierPredict                   m_onPredict;
    ofstream                            m_lstream;
    streambuf                           *m_origStderr;


    __gnu_cxx::hash_map<uint32_t, std::string> m_categoryIndex;

public:
    Classifier(const char *filename, const char *option = NULL);
    ~Classifier();
    void train(const char *filename);
    int classify(const char *document, const char **classname);
    int classify(const char *document, const double **probs);
    const char* getClassNameById(int id);
};

} //zdcp
#endif // CLASSIFIER_H_
// vim: ts=4 sw=4 ai cindent et
