#ifndef DOC_PARSER_FACTORY_H_
#define DOC_PARSER_FACTORY_H_

#include <stdexcept>

#include "DocParser.h"
#include "DcpParser.h"

#include <common/Factory.h>

#define REGISTER_PARSER(N) if ( strcmp( name, #N ) == 0 ) \
        return new N##Parser();

namespace zdcp {

class DocParserFactory:public Factory<DocParserFactory, DocParser> {

protected:    
    DocParser *createProduct(const char *name)
    {
        REGISTER_PARSER(Dcp);
        THROW_RUNTIME_ERROR(std::string("unknow ") + getFactoryName()
                + " name : " + std::string(name));
    }

public:
    std::string getFactoryName()
    {
        return "document parser";
    }

};

} // zdcp

#undef REGISTER_PARSER                   

#endif // DOC_PARSER_FACTORY_H_
// vim: ts=4 sw=4 ai cindent et
