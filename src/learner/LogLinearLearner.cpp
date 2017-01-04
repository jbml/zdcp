#include <algorithm>
#include <cassert>
#include <fstream>
#include <set>
#include <cmath>

#include "LogLinearLearner.h"

namespace zdcp {
    using namespace std;

    const string LogLinearLearner::getName()
    {
        return "LogLinear";
    }

    const string LogLinearLearner::getVersion()
    {
        return "1.0";
    }

    void LogLinearLearner::initTrain(DocumentList& docList){
        m_nonZero = 0;
        m_numAttr = 0;
        m_docWeightSum = 0.0;
        m_numClass = docList.CategoryIndex.size();

        vector< set<int> > lambdaVecTmp; // lambda terms occurring in classes

        //
        for (vector<Document>::const_iterator it = docList.Documents.begin(); it != docList.Documents.end(); ++it){
            const Document &doc = *it;
            for (DocumentFeature::const_iterator featureIt = doc.Feature.begin(); featureIt != doc.Feature.end(); ++featureIt){
                int tid = featureIt->first;
                if(m_numAttr < tid){
                    m_numAttr = tid;
                }
            }
        }// for
        lambdaVecTmp.resize(m_numAttr + 1);

        //
        for (vector<Document>::const_iterator it = docList.Documents.begin(); it != docList.Documents.end(); ++it){
            const Document &doc = *it;
            for (DocumentFeature::const_iterator featureIt = doc.Feature.begin(); featureIt != doc.Feature.end(); ++featureIt){
                int tid = featureIt->first;
                if (lambdaVecTmp[tid].find(doc.Category) == lambdaVecTmp[tid].end()){
                    lambdaVecTmp[tid].insert(doc.Category);
                    // doesn't matter what it is , we start from 0 nonetheless, this is just to know what to update in sparse
                    m_nonZero++;
                }
            }
            m_docWeightSum += doc.Weight;
        }// for
		if(m_docWeightSum < 1){
			m_docWeightSum = m_numAttr;
		}

        m_lambdaTInC.clear();
        m_lambdaTInC.resize(m_numAttr + 1);
        int count = 0;
        for (int i = 1; i <= m_numAttr; i++){
            for (set<int>::const_iterator it = lambdaVecTmp[i].begin(); it != lambdaVecTmp[i].end(); ++it){
                int classIndex = *it;
                m_lambdaTInC[i].push_back(pair<int, int>(classIndex, count++)); // count is the offset
            }
        }
    }

    double LogLinearLearner::dot(const DocumentFeature& docFeature, const vector<double>& lambda)
    {
        double sum = 0;

        for (DocumentFeature::const_iterator it = docFeature.begin(); it != docFeature.end(); it++){
            uint32_t tid = (*it).first;

            if ( tid > 0 && tid < lambda.size()){
                double v = (*it).second * lambda[tid];
                sum += v;
            }
        }

        return sum;
    } // Dot2

    int LogLinearLearner::getAllProbs(const DocumentFeature& docFeature,bool predict){
        // computing prod 
        double maxLog = -HUGE_VAL;
        int argmaxClassIndex = 0;

        vector<double> prodVecTmp;
        prodVecTmp.resize(m_numClass + 1, 0);

        m_prodVec.clear();
        m_prodVec.resize(m_numClass + 1, 0);	

        if(!predict){
            for (DocumentFeature::const_iterator it = docFeature.begin(); it != docFeature.end(); it++){
                int tid = (*it).first;
                for (vector< pair<int,int> >::iterator lambdaIt = m_lambdaTInC[tid].begin(); lambdaIt != m_lambdaTInC[tid].end(); ++lambdaIt){
                    double valTmp = (*it).second * m_gradientLambdas[(*lambdaIt).second];
                    int cid = (*lambdaIt).first;
                    prodVecTmp[cid] += valTmp;
                }
            }
            for (int i = 1; i <= m_numClass; i++){
                if (prodVecTmp[i] != 0){
                    if (prodVecTmp[i] > maxLog){
                        maxLog = prodVecTmp[i];
                        argmaxClassIndex = i;
                    }
                }
            }
        }else{
            for (int i = 1; i <= m_numClass; i++){
                prodVecTmp[i] = dot(docFeature, m_lambdaVec[i]); // P(c | x)

                if (prodVecTmp[i] != 0 && prodVecTmp[i] > maxLog){
                    maxLog = prodVecTmp[i];
                    argmaxClassIndex = i;
                }
            }
        }

        //
        double norm = 0;
        for (int i = 1; i <= m_numClass; i++){
            if (prodVecTmp[i] != 0){
                m_prodVec[i] = exp(prodVecTmp[i] - maxLog);
                norm += m_prodVec[i];
            }
        }
        if (norm > 0){
            for (int i = 1; i <= m_numClass; i++){
                m_prodVec[i] /= norm;
            }
        }
        else{
            for (int i = 1; i <= m_numClass; i++){
                m_prodVec[i] = 1.0/m_numClass;
            }
        }

        return argmaxClassIndex;
    }

    //
    pair<double,double> LogLinearLearner::getGradient(double *gradients,DocumentList& docList){
        // returns likelihood
        double likelihood = 0.0;
        double right = 0.0;

        // this computes the gradient on the current iteration BEGIN
        for (vector<Document>::const_iterator it = docList.Documents.begin(); it != docList.Documents.end(); ++it){
            const Document &doc = *it;
            int probClassIndex = getAllProbs(doc.Feature); // this is going to use lambda!
            int targetClassIndex = doc.Category;

            likelihood += doc.Weight*log(m_prodVec[targetClassIndex]);
            right += (targetClassIndex == probClassIndex) ?  doc.Weight : 0;
            for (DocumentFeature::const_iterator featureIt = doc.Feature.begin(); featureIt != doc.Feature.end(); ++featureIt){
                int tid = featureIt->first;
                for (vector< pair<int,int> >::iterator lambdaIt = m_lambdaTInC[tid].begin(); lambdaIt != m_lambdaTInC[tid].end(); ++lambdaIt){
                    int cid = (*lambdaIt).first;
                    gradients[(*lambdaIt).second] -= doc.Weight * featureIt->second * (((cid == targetClassIndex) ? 1 : 0) - m_prodVec[cid]);
                }
            }
        }// for

        return pair<double,double>(-likelihood, right/m_docWeightSum);
        // this computes the gradient on the current iteration END
    }

    void LogLinearLearner::initLambdaVec(vector< vector<double> >&lambdaVec,int numClass,int numAttr){
        lambdaVec.clear();
        lambdaVec.resize(numClass + 1);

        for(int i = 1; i <= numClass; i++){
            lambdaVec[i].resize(numAttr + 1,0.0);
        }
    }

    void LogLinearLearner::transpose(){
        initLambdaVec(m_lambdaVec,m_numClass,m_numAttr);

        for (int i = 1; i <= m_numAttr; i++){
            for (vector< pair<int,int> >::iterator lambdaIt = m_lambdaTInC[i].begin(); lambdaIt != m_lambdaTInC[i].end(); ++lambdaIt){
                int classIndex = (*lambdaIt).first;
                int lambIndex = (*lambdaIt).second;

                if(classIndex < 1 || classIndex > m_numClass)
                    THROW_RUNTIME_ERROR("Class index abnormity");
                if(lambIndex < 0 || lambIndex >= m_gradeientLmbSize)
                    THROW_RUNTIME_ERROR("Lambda index abnormity");

                m_lambdaVec[classIndex][i] = m_gradientLambdas[lambIndex];
            }
            m_lambdaTInC[i].clear();
        }
        delete[] m_gradientLambdas;
        m_gradientLambdas = NULL;
    }

    void LogLinearLearner::doGradient(DocumentList& docList){

        m_gradeientLmbSize = m_nonZero;
        m_gradientLambdas = new double[m_gradeientLmbSize];
        double *gradients = new double[m_gradeientLmbSize];
        LBFGS *lbfgs = new LBFGS(m_gradeientLmbSize);

        bzero(m_gradientLambdas, m_gradeientLmbSize*sizeof(double));
        cerr << "Parameters estimation ..." << endl;

        //
        double obj = 0, objPrev;
        double rightTrn = 0, rightTrnPrev;
        int ret, iteration = 0;
        int iterRelErrorCondition = 0;
        int iterRightCondition = 0;
        double gaussSigmaSquare = m_param.gaussSigmaConst * m_param.gaussSigmaConst;
        do{
            // this is the swap, should only map for nCycles > 0; storing the old direction here...
            objPrev = obj;
            rightTrnPrev = rightTrn;

            bzero(gradients, m_gradeientLmbSize*sizeof(double)); 

            pair<double,double> likelihoodRight = getGradient(gradients,docList); //LBFGS minimizes objective function. We want to maximize -log likelihood
            obj = likelihoodRight.first;
            rightTrn = likelihoodRight.second;

            for (int i = 0 ; i < m_gradeientLmbSize; ++i){
                gradients[i] += m_gradientLambdas[i]/gaussSigmaSquare;
                obj += 0.5*m_gradientLambdas[i]*m_gradientLambdas[i]/gaussSigmaSquare;
            }

            ret = lbfgs->optimize(m_gradientLambdas, &obj, gradients);

            double currRelError = fabs((obj - objPrev)/obj);
            iterRelErrorCondition += (currRelError <= m_param.relErrorLimit && obj < objPrev);
            int bnRightIncrease = (rightTrn > rightTrnPrev) ? 1 : 0;
            if (!bnRightIncrease){
                iterRightCondition++;
            }
            else{
                iterRightCondition = 0;
            }

            cerr << "#iter = " << iteration << endl;
            cerr << "LogLikelihood = " << obj << "; return value = " << ret << endl;
            cerr << "curr.rel.value = " << currRelError << "; nRightTrn = " << rightTrn << endl;
            cerr << "nRightTrn increased = " << bnRightIncrease << endl;
            cerr << "number of iterations without up to expectations= " << iterRelErrorCondition << endl;
            cerr << "number of iterations without improvement = " << iterRightCondition << endl;
            cerr << endl;

            iteration++;

        }while(ret != 0 && iterRelErrorCondition < m_param.iterRelErrorLimit && iterRightCondition < m_param.iterRightLimit && iteration < m_param.iteratorLimit);
        if(ret == 0){
            cerr << "Optimization finished: error inside lbfgs -- quasi-Newton package...";
        }else if(iterRelErrorCondition >= m_param.iterRelErrorLimit){
            cerr << "Optimization finished: up to the limiting number of iterations exceeded relation error" << endl;
        }else if(iterRightCondition >= m_param.iterRightLimit){
            cerr << "Optimization finished: up to the limiting number of iterations without improvement" << endl;
        }else{
            cerr << "Optimization finished: up to the total limiting number of iterations" << endl;
        }

        delete[] gradients;
        delete lbfgs;

        transpose(); // populating the lambda structure in case of sparse gradient
    }

    void LogLinearLearner::learn(DocumentList &docList)
    {
        if(docList.CategoryIndex.size() < 2){
            THROW_RUNTIME_ERROR("Cannot train because only include one class in training data set");
        }
        initTrain(docList);
        doGradient(docList);
        saveModel(m_param.modelFile);
    }

    void LogLinearLearner::predict(DocumentList &docList, OnPredict &onPredict)
    {
        //load model
        loadModel(m_param.modelFile);//THROW_RUNTIME_ERROR exception

        //preidct
        if (!m_probabilities)
            m_probabilities = new double[m_numClass];

        for (size_t i = 0; i < docList.Documents.size(); i++) {
			int maxProbClassIndex = 0;

			//calc probabilities
            if (docList.Documents[i].Feature.size() != 0){
				getAllProbs(docList.Documents[i].Feature,true);

				double maxProb = 0.0;
				for (int j = 1; j <= m_numClass; j++){
					int tmpProbIndex = j - 1;
					m_probabilities[tmpProbIndex] = m_prodVec[j];
					if(maxProb < m_probabilities[tmpProbIndex]){
						maxProb = m_probabilities[tmpProbIndex];
						maxProbClassIndex = j;
					}
				}
			}
            onPredict(maxProbClassIndex, m_probabilities,m_numClass);
        }
    }

    bool LogLinearLearner::saveModel(string fileName)
    {
        fstream f;

        cerr << "Writing LogLinearLearner Model..." << endl;
        f.open(fileName.c_str(), ios::out | ios::trunc);

        if (f.rdstate() == ios::goodbit){
            cerr << "Opened file " << fileName.c_str() << " fine..." << endl;

            f << *this;
            f.close();
            cerr << "Done writing." << endl;
            return 1;
        }

        f.close();
        cerr << "Open file" << fileName.c_str() << " fail..." << endl;
        cerr << "Writing interrupt." << endl;
        return 0;
    }

    bool LogLinearLearner::loadModel(string fileName)
    {
        if (m_modelLoaded) return true;

        fstream f;

        cerr << "Loading the Model..." << endl;
        f.open(fileName.c_str(), ios::in);    

        if (f.rdstate() == ios::goodbit){
            cerr << "Reading the Model " << fileName.c_str() << " ... " << endl;
            try{
                f >> *this;
                cerr << "Done Reading." << endl;
                m_modelLoaded = true;
            }catch(runtime_error err){
                cerr << "Reading interrupt:" << err.what() << endl;
            }

            f.close();
            return 1;
        }

        f.close();
        cerr << "Open file" << fileName.c_str() << " fail..." << endl;
        cerr << "Loading interrupt." << endl;
        return 0;
    }

    void LogLinearLearner::put(ostream &out) const
    {
        out << "gauss_sigma_const " << m_param.gaussSigmaConst << endl;
        out << "rel_error_limit " << m_param.relErrorLimit << endl;
        out << "iter_rel_error_limit " << m_param.iterRelErrorLimit << endl;
        out << "iter_right_limit " << m_param.iterRightLimit << endl;
        out << "iter_limit " << m_param.iteratorLimit << endl;
        out << "nonzeror_val " << m_nonZero << endl;

        out << "attr_num " << m_numAttr << endl;
        out << "class_num " << m_numClass << endl;
        out << "param_esti " << "LBFGS" << endl;

        //lambda matrix
        for (int i = 1; i <= m_numClass; i++){
            for (uint32_t j = 1; j < m_lambdaVec[i].size(); j++){
                out << m_lambdaVec[i][j] << " ";
            }
            out << endl;
        }
    }

    void LogLinearLearner::get(istream &in)
    {
        string itemName,itemValue;

        in >> itemName >> m_param.gaussSigmaConst;
        if(itemName.compare("gauss_sigma_const") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(gauss_sigma_const)");
        }
        in >> itemName >> m_param.relErrorLimit;
        if(itemName.compare("rel_error_limit") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(rel_error_limit)");
        }
        in >> itemName >> m_param.iterRelErrorLimit;
        if(itemName.compare("iter_rel_error_limit") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(iter_rel_error_limit)");
        }
        in >> itemName >> m_param.iterRightLimit;
        if(itemName.compare("iter_right_limit") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(iter_right_limit)");
        }
        in >> itemName >> m_param.iteratorLimit;
        if(itemName.compare("iter_limit") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(iter_limit)");
        }
        in >> itemName >> m_nonZero;
        if(itemName.compare("nonzeror_val") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(nonzeror_val)");
        }

        in >> itemName >> m_numAttr;
        if(itemName.compare("attr_num") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(attr_num)");
        }
        in >> itemName >> m_numClass;
        if(itemName.compare("class_num") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(class_num)");
        }
        in >> itemName >> itemValue;
        if(itemName.compare("param_esti") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(param_esti)");
        }else if(itemValue.compare("LBFGS") != 0){
            THROW_RUNTIME_ERROR("Unknown log-linear parameters estimate alg");
        }

        //lambda matrix
        initLambdaVec(m_lambdaVec,m_numClass,m_numAttr);

        for (int i = 1; i <= m_numClass; i++){
            for(int j = 1; j <= m_numAttr; j++){
                if (in.rdstate() != ios::goodbit){
                    THROW_RUNTIME_ERROR("Irregular model file format(lambdas)");
                }
                double lambda = 0.0;
                in >> lambda;
                m_lambdaVec[i][j] = lambda;
            }
        }
    }

    ostream& operator<< (ostream& out, const LogLinearLearner &logLinearLearner)
    {
        out.precision(16);//double precision
        out.setf(ios::fixed);
        out << "name " << LogLinearLearner::getName() << endl;
        out << "version " << LogLinearLearner::getVersion() << endl;

        logLinearLearner.put(out);
        return out;
    }

    istream& operator>> (istream &in, LogLinearLearner &logLinearLearner)
    {
        string itemName,itemValue;

        in >> itemName >> itemValue;
        if(itemName.compare("name") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(name)");
        }else if(itemValue.compare(LogLinearLearner::getName()) != 0){
            THROW_RUNTIME_ERROR("Not log-linear model");
        }

        in >> itemName >> itemValue;
        if(itemName.compare("version") != 0){
            THROW_RUNTIME_ERROR("Irregular model file format(version)");
        }else if(itemValue.compare(LogLinearLearner::getVersion()) != 0){
            THROW_RUNTIME_ERROR("Unknown model file version");
        }

        logLinearLearner.get(in);
        return in;
    }
} //zdcp
// vim: ts=4 sw=4 et ai cindent
