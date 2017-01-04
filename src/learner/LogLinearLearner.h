#ifndef LOG_LINEAR_H_
#define LOG_LINEAR_H_

#include <string>

#include "config/SimpleConfig.h"
#include "document/DocumentList.h"
#include "liblbfgs/LBFGS.h"

#include "Learner.h"

namespace zdcp {

using namespace std;
    
class LogLinearLearner:public Learner {
public:
	typedef struct Parameters
	{
		string modelFile;
		double gaussSigmaConst;
		double relErrorLimit;
		int iterRelErrorLimit;
		int iterRightLimit;
		int iteratorLimit;
	} TParameters;
protected:
	//configure parameters
	struct Parameters m_param;
	
	//in-between various
	double* m_gradientLambdas;
	int m_gradeientLmbSize;
	double m_docWeightSum;
	int m_nonZero;
	vector< vector< pair<int,int> > > m_lambdaTInC;
	vector<double> m_prodVec;
	
	//target
	int m_numAttr;
	int m_numClass;
	vector< vector<double> > m_lambdaVec;
    bool m_modelLoaded;
    double *m_probabilities;
protected:
	void initTrain(DocumentList& docList);
	double dot(const DocumentFeature& docFeature, const vector<double>& lambda);
	int getAllProbs(const DocumentFeature& docFeature,bool predict = false);
	pair<double,double> getGradient(double *gradients,DocumentList& docList);
	void transpose();
	void doGradient(DocumentList& docList);
	bool saveModel(std::string fileName);
	bool loadModel(std::string fileName);
	void initLambdaVec(vector< vector<double> >&lambdaVec,int numClass,int numAtrr);

public:
	LogLinearLearner()
        :m_modelLoaded(false),m_probabilities(NULL)
	{
        // default params
        m_param.modelFile = "loglinear.model";
        m_param.gaussSigmaConst = 10.0;
        m_param.relErrorLimit = 0.001;
        m_param.iterRelErrorLimit = 2;
        m_param.iterRightLimit = 1000;
        m_param.iteratorLimit = 10;

		//
		m_gradientLambdas = NULL;
		m_gradeientLmbSize = 0;
		m_docWeightSum = 0.0;
		m_nonZero = 0;
		m_numAttr = 0;
		m_numClass = 0;
	}

	~LogLinearLearner()
	{
		if (m_gradientLambdas != NULL){
			delete[] m_gradientLambdas;
			m_gradientLambdas =  NULL;
		}

        if (m_probabilities)
            delete [] m_probabilities;
	}

	void configure(SimpleConfig &config)
	{
		Learner::configure(config);        

		config.getValue("loglinear.modelfile", m_param.modelFile);
		config.getValue("loglinear.gauss_sigma_const", m_param.gaussSigmaConst);
		config.getValue("loglinear.rel_error_limit", m_param.relErrorLimit);
		config.getValue("loglinear.iter_rel_error_limit", m_param.iterRelErrorLimit);
		config.getValue("loglinear.iter_right_limit", m_param.iterRightLimit);
		config.getValue("loglinear.iter_limit", m_param.iteratorLimit);
	}

	void put(std::ostream &out) const;
	void get(std::istream &in);

	static const string getName();
	static const string getVersion();
	void learn(DocumentList &docList);//throw runtime_error
	void predict(DocumentList &docList, OnPredict &onPredict);//throw runtime_error
};

ostream& operator<< (ostream& out, const LogLinearLearner &logLinearLearner);
istream& operator>> (istream &in, LogLinearLearner &logLinearLearner);


} // zdcp;

#endif // LOG_LINEAR_H_

// vim: ts=4 sw=4 et ai cindent
