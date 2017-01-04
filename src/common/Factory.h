#ifndef FACTORY_H_
#define FACTORY_H_

#include <string>
#include <map>
#include <stdexcept>

namespace zdcp {

template<class factoryType, class productType>
class Factory {

protected:
    // Why cannot use productType * ?
    typedef std::map<std::string, void *> InstanceTable;
    InstanceTable  m_instanceTable;

    virtual productType *createProduct(const char *name) = 0;

public:
    // We may use more than one zdcp in same process, 
    // so do not make factories singleton
    Factory()
    {
    }

    virtual ~Factory()
    {
        InstanceTable::iterator it;

        for (it = m_instanceTable.begin(); it != m_instanceTable.end(); it++) {
            if (it->second)
                delete static_cast<productType *>(it->second);
        }
        
    }

    virtual std::string getFactoryName() = 0;
    productType *create(const char *name) 
    {
        std::string s(name);
        InstanceTable::iterator it;

        it = m_instanceTable.find(name);
        if (it == m_instanceTable.end()) {
            m_instanceTable[name] = NULL;
            if ((it = m_instanceTable.find(name)) == m_instanceTable.end()) 
               THROW_RUNTIME_ERROR(std::string("this program has been kiced by a donkey!")); 
        }

        if (it->second == NULL) {
            it->second = createProduct(name);
        }

        return static_cast<productType *>(it->second);

    }
};

} // zdcp

#endif // FACTORY_H_
// vim: ts=4 sw=4 ai cindent et
