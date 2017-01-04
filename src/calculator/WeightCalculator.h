#ifndef WEIGHT_CALCULATOR_H_
#define WEIGHT_CALCULATOR_H_

#include <stdint.h>
#include <map>
#include <string>
#include <ext/hash_map>

#include "config/SimpleConfig.h"
#include "feature/FeatureSelector.h"
#include "document/DocumentList.h"

namespace zdcp {

class WeightCalculator {
protected:
    std::map<std::string, double>   m_fieldWeight;
    std::vector<std::string>        m_documentFields;
    
public:
	virtual void calculate(DocumentList &docList, const Feature &feature)=0;
    virtual void configure(SimpleConfig &config)
    {
        std::vector<std::string>::iterator    fieldIt;

        config.getValue("document.fields", m_documentFields);

        for (fieldIt = m_documentFields.begin();
             fieldIt != m_documentFields.end();
             fieldIt++)
        {
        	std::string  prefix = "weight.";
            config.getValue(prefix.append(*fieldIt), m_fieldWeight[*fieldIt]);
        }
    }
	virtual ~WeightCalculator()
	{
	}
};

} // zdcp

#endif //WEIGHT_CALCULATOR_H_
