#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Classifier.h"

namespace zdcp {

static void trieExport(const char *s, int val, void *puser) 
{
    __gnu_cxx::hash_map<uint32_t, std::string> *index;
    
    index = static_cast<__gnu_cxx::hash_map<uint32_t, std::string> *>(puser);
//    std::cout << "val = " << val << " s = " << s << std::endl;
    (*index)[(uint32_t)val].assign(s);
}

Classifier::Classifier(const char *filename, const char *option)
    :m_config(filename)
{
    std::string                         parserName;
    std::string                         selectorName;
    std::string                         calculatorName;
    std::string                         learnerName;
    std::string                         categoryIndexFile;
    std::string                         logFileName;

    if (option)
        m_config << option;

    m_config.getValue("document.parser", parserName);
    m_config.getValue("feature.selector", selectorName);
    m_config.getValue("weight.calculator", calculatorName);
    m_config.getValue("training.learner", learnerName);
    m_config.getValue("index.category", categoryIndexFile);
    m_config.getValue("log.file", logFileName);

    if (!logFileName.empty()) {
        m_lstream.open(logFileName.c_str(), std::ios_base::app);
        if (m_lstream.good()) {
            m_origStderr = std::cerr.rdbuf();
            std::cerr.rdbuf(m_lstream.rdbuf());
        } else {
            std::cerr << "failed to open log file " << logFileName << std::endl;
        }
    }

    m_dpFactory = new DocParserFactory();
    m_parser = m_dpFactory->create(parserName.c_str());
    m_parser->configure(m_config);

    m_fsFactory = new FeatureSelectorFactory();
    m_selector = m_fsFactory->create(selectorName.c_str());
    m_selector->configure(m_config);

    m_wcFactory = new WeightCalculatorFactory();
    m_calculator = m_wcFactory->create(calculatorName.c_str());
    m_calculator->configure(m_config);

    m_lrFactory = new LearnerFactory();
    m_learner = m_lrFactory->create(learnerName.c_str());
    m_learner->configure(m_config);

    struct stat sb;
    if (stat(categoryIndexFile.c_str(), &sb) == 0) {
        Trie    categoryTrie(categoryIndexFile.c_str());
        categoryTrie.explore(trieExport, &m_categoryIndex);
    }

    double threshold;
    m_config.getValue("predictor.threshold", threshold);
    std::cerr << "prediction threshold = " << threshold << std::endl;
}

void Classifier::train(const char *filename) 
{
    DocumentList                        docList;
    Feature                             feature;

    // Parse document
    m_parser->parseFile(filename, docList, PM_UNKNOW);
    // Feature Select
    m_selector->select(docList, feature);
    // Weight Calculate
    m_calculator->calculate(docList, feature);
    // train model
    m_learner->learn(docList);
}

int Classifier::classify(const char *document, const char **classname)
{
    DocumentList                        docList;
    Feature                             feature;

    // Parse document
    m_parser->parseText(document, docList, PM_LOAD);
    // Weight Calculate
    m_calculator->calculate(docList, feature);
    // Predict document
    m_onPredict.configure(m_config);
    m_learner->predict(docList, m_onPredict);
    if (m_categoryIndex.size() == 0)
        std::cerr << "warning category index size is zero!" << std::endl;
    // std::cout << "cid = " << m_onPredict.getClassId() << std::endl;
    if(classname!=NULL){
        *classname = m_categoryIndex[m_onPredict.getClassId()].c_str();
    }

    return m_onPredict.getClassId();
}

int Classifier::classify(const char *document, const double **probs)
{
    DocumentList                        docList;
    Feature                             feature;

    // Parse document
    m_parser->parseText(document, docList, PM_LOAD);
    // Weight Calculate
    m_calculator->calculate(docList, feature);
    // Predict document
    m_onPredict.configure(m_config);
    m_learner->predict(docList, m_onPredict);

    *probs = m_onPredict.getProbs();

    return m_onPredict.getNumClass();
}

const char* Classifier::getClassNameById(int id)
{
    if (m_categoryIndex.size() == 0)
        std::cerr << "warning category index size is zero!" << std::endl;
    if (id == 0)
        return "*";
    return m_categoryIndex[id].c_str();
}

Classifier::~Classifier()
{
    if (m_lstream.is_open()) {
        std::cerr.rdbuf(m_origStderr);
        m_lstream.close();
    }
    delete m_fsFactory;
    delete m_dpFactory;
    delete m_wcFactory;
    delete m_lrFactory;
}

} //zdcp
// vim: ts=4 sw=4 ai cindent et
