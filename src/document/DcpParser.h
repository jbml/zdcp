#ifndef DCP_PARSER_H_
#define DCP_PARSER_H_

#include <map>
#include <vector>
#include <string>
#include <stdexcept>

#include "DocParser.h"
#include "bamboo/bamboo.hxx"

#define BUFFER_MAX 1024 * 1024 * 32

namespace zdcp {
class DcpParser: public DocParser {
public:
    DcpParser():DocParser()
    {
    }
    virtual void parseText(const char *text, DocumentList &docList, ParseMode mode);
    virtual void parseFile(const char *filename, DocumentList &docList, ParseMode mode);
};

} // zdcp

#endif // DCP_PARSER_H_

// vim: ts=4 sw=4 ai cindent et
