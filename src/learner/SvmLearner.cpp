#include <algorithm>
#include <cassert>
#include "SvmLearner.h"

namespace zdcp {

static bool feature_compare(const std::pair<uint32_t, double> &lhs,
                            const std::pair<uint32_t, double> &rhs)
{
    return lhs.first < rhs.first;
}

void SvmLearner::sortFeature(std::vector<std::pair<uint32_t, double> > &featureVector, DocumentFeature &feature)
{
    DocumentFeature::iterator             featureIt;

    for (featureIt = feature.begin();
            featureIt != feature.end();
            featureIt++)
    {
        featureVector.push_back(std::make_pair(featureIt->first, featureIt->second));
    }

    std::sort(featureVector.begin(), featureVector.end(), feature_compare); 
}

void SvmLearner::doCrossValidation(struct svm_problem prob)
{
    // copy from libsvm svm-train.c
    int i;
    int total_correct = 0;
    double total_error = 0;
    double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
    double *target = new double[prob.l];

    svm_cross_validation(&prob,&m_param,m_fold,target);
    if(m_param.svm_type == EPSILON_SVR ||
            m_param.svm_type == NU_SVR)
    {
        
        for(i=0;i<prob.l;i++)
        {
            double y = prob.y[i];
            double v = target[i];
            total_error += (v-y)*(v-y);
            sumv += v;
            sumy += y;
            sumvv += v*v;
            sumyy += y*y;
            sumvy += v*y;
        }
        std::cerr << "Cross Validation Mean squared error = "
            << total_error/prob.l << std::endl;

        std::cerr << "Cross Validation Squared correlation coefficient = "
            << ((prob.l*sumvy-sumv*sumy)*(prob.l*sumvy-sumv*sumy))/
            ((prob.l*sumvv-sumv*sumv)*(prob.l*sumyy-sumy*sumy)) << std::endl;

    } else {

        for(i=0;i<prob.l;i++)
            if(target[i] == prob.y[i])
                ++total_correct;

        std::cout << "Cross Validation Accuracy = " 
                  << 100.0 * total_correct / prob.l
                  << "%" << std::endl;

    }
    delete []target;
}

void SvmLearner::learn(DocumentList &docList)
{
    // copy from libsvm svm-train.c
    uint32_t                                      cnt;
    std::vector<Document>::iterator               it;
	struct svm_model                              *model;
	struct svm_problem                            prob;
    struct svm_node                               *x_space;
    int32_t                                       max_index;
    size_t                                        num_space;
    const char *                                  error_msg;
    int                                           i, j;


    cnt = 1;

    prob.l = docList.Documents.size();

    prob.y = new double[prob.l];
    prob.x = new struct svm_node *[prob.l];

    num_space = 0;
	for (i = 0; i < prob.l; i++)
        num_space += docList.Documents[i].Feature.size() + 1;

    x_space = new struct svm_node[num_space];

    max_index = 0;
    for (i = 0, j = 0; i < prob.l; i++) { 
        std::vector<uint32_t>::iterator       termIt;
        
        prob.x[i] = &x_space[j];
        prob.y[i] = docList.Documents[i].Category;

        if (m_showVector)
            std::cout << prob.y[i] << " ";

        std::vector<std::pair<uint32_t, double> > featureVector; 
        sortFeature(featureVector, docList.Documents[i].Feature);

        std::vector<std::pair<uint32_t, double> >::iterator featureIt; 

        for (featureIt = featureVector.begin();
             featureIt != featureVector.end();
             featureIt++)
        {
            x_space[j].index = featureIt->first;
            x_space[j].value = featureIt->second;

            if (m_showVector)
                std::cout << x_space[j].index << ":" << x_space[j].value << " ";

            ++j;
        }

#undef TERMS
        if (j >= 1 && x_space[j - 1].index > max_index)
            max_index = x_space[j - 1].index;
        x_space[j++].index = -1;
        
        if (m_showVector)
            std::cout << std::endl;

        cnt++;
    }

    if (m_param.gamma == 0)
        m_param.gamma = 1.0 / max_index;

    if (m_param.kernel_type == PRECOMPUTED) {
        for (i = 0; i < prob.l; i ++) {
            if (prob.x[i][0].index != 0)
                THROW_RUNTIME_ERROR("Wrong input data!");
            if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
                THROW_RUNTIME_ERROR("Wrong input data!");
        }
    }


    if ((error_msg = svm_check_parameter(&prob, &m_param))) {
        std::cout << error_msg << std::endl;
        THROW_RUNTIME_ERROR(std::string("Error: ") + error_msg);
    }

    if (!m_showVector) {
        model = svm_train(&prob, &m_param);
        svm_save_model(m_modelFile.c_str(), model);
        svm_destroy_param(&m_param);
        if (m_fold > 0)
            doCrossValidation(prob);
        svm_destroy_model(model);
    }


    delete []prob.y;
    delete []prob.x;
    delete []x_space;
}

void SvmLearner::predict(DocumentList &docList, OnPredict &onPredict)
{
    // copy from libsvm svm-predict.c
    size_t                        i;
    size_t                        numDocs;
    struct svm_node               *x_space;
    size_t                        num_space;
    size_t                        j;
    double                        *probabilities = NULL;

    numDocs = docList.Documents.size();

    num_space = 0;
	for (i = 0; i < numDocs; i++) {
        if (num_space < docList.Documents[i].Feature.size())
            num_space = docList.Documents[i].Feature.size();
    }
    
    num_space++;

    x_space = new struct svm_node[num_space];


    m_predictModel = getModel();
	if (!m_probabilities)
		m_probabilities = new double[svm_get_nr_class(m_predictModel)];

    for (i = 0; i < numDocs; i++) { 
		uint32_t cid = 0;
        if (docList.Documents[i].Feature.size() != 0){

			std::vector<std::pair<uint32_t, double> > termVector; 
			sortFeature(termVector, docList.Documents[i].Feature);

			j = 0;

			std::vector<std::pair<uint32_t, double> >::iterator termIt; 

			for (termIt = termVector.begin();
				 termIt != termVector.end();
				 termIt++)
			{
				x_space[j].index = termIt->first;
				x_space[j].value = termIt->second;

				if (m_showVector)
					std::cout << x_space[j].index << ":" << x_space[j].value << " ";

				++j;
			}

	#undef TERMS       

			if (m_showVector)
				std::cout << std::endl;

			x_space[j].index = -1;
			x_space[j].value = -1;
			

			cid = static_cast<uint32_t>(
						svm_predict_probability(m_predictModel, x_space, m_probabilities)
						);
		}
         onPredict(cid, m_probabilities,svm_get_nr_class(m_predictModel));

    }

    delete [] probabilities;
    delete []x_space;
}


} //zdcp
// vim: ts=4 sw=4 et ai cindent
