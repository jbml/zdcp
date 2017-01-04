#ifndef DOCUMENT_LIST_H_
#define DOCUMENT_LIST_H_

#include <stdint.h>

#include <map>
#include <string>

#include "Document.h"

namespace zdcp {

typedef __gnu_cxx::hash_map<std::string, uint32_t, str_hash, str_equal> HashIndex;

class DocumentList {
public:
    HashIndex                   CategoryIndex;
    HashIndex                   TermIndex;
    std::vector<Document>       Documents;
};


} // zdcp
#endif //DOCUMENT_LIST_H_

// vim: ts=4 sw=4 ai cindent et
