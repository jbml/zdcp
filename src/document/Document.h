#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <stdint.h>

#include <map>
#include <string>

#include "common/hash.h"


namespace zdcp {

typedef std::vector<uint32_t> TermList;
typedef __gnu_cxx::hash_map<uint32_t, double> DocumentFeature;

class Document {
public:
    uint32_t                        Category;
    DocumentFeature                 Feature;
    std::map<std::string, TermList> TermIds;
	int								Weight;

public:
	Document():Weight(1){};
};


} // zdcp
#endif //DOCUMENT_H_

// vim: ts=4 sw=4 ai cindent et
