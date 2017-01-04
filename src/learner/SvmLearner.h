#ifndef SVM_LEARNER_H_
#define SVM_LEARNER_H_

#include <string>

#include "libsvm/svm.h"
#include "config/SimpleConfig.h"
#include "document/DocumentList.h"

#include "Learner.h"

namespace zdcp {
    
class SvmLearner:public Learner {
protected:
    std::string             m_modelFile;
	struct svm_parameter    m_param;
    struct svm_model        *m_predictModel;
    int                     m_fold;
	double *m_probabilities;

    void sortFeature(std::vector<std::pair<uint32_t, double> > &featureVector,
                     DocumentFeature &feature); 
    void doCrossValidation(struct svm_problem prob);

    struct svm_model *getModel() 
    {
        if (!m_predictModel)
            m_predictModel = svm_load_model(m_modelFile.c_str());

        return m_predictModel;
    }
public:
   SvmLearner(): m_predictModel(NULL), m_fold(0)
   {
        // default params
        m_param.svm_type = C_SVC;
        m_param.kernel_type = RBF;
        m_param.gamma = 0;
        m_param.C = 1;
        m_param.degree = 3;
        m_param.coef0 = 0;
        m_param.nu = 0.5;
        m_param.cache_size = 100;
        m_param.eps = 1e-3;
        m_param.p = 0.1;
        m_param.shrinking = 1;
        // output probability
        m_param.probability = 1;
        m_param.nr_weight = 0;
        m_param.weight_label = NULL;
        m_param.weight = NULL;
   }
   
   ~SvmLearner()
   {
       if (m_predictModel) 
           svm_destroy_model(m_predictModel);
	    if (m_probabilities)
            delete []m_probabilities;
   }

   void configure(SimpleConfig &config)
   {
        Learner::configure(config);
        

        config.getValue("svm.modelfile", m_modelFile);
        config.getValue("svm.gamma", m_param.gamma);
        config.getValue("svm.C", m_param.C);
        config.getValue("svm.cache_size", m_param.cache_size);
        config.getValue("svm.fold", m_fold);

   }
   void learn(DocumentList &docList);
   void predict(DocumentList &docList, OnPredict &onPredict);
};

} // zdcp;

#endif // SVM_LEARNER_H_

// vim: ts=4 sw=4 et ai cindent
