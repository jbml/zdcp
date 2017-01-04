#ifndef HASH_H_
#define HASH_H_

#include <cstring>
#include <ext/hash_map>

namespace zdcp {

struct str_hash {
    size_t operator()(const std::string &s) const
    {
        // ref: http://www.cse.yorku.ca/~oz/hash.html
        
        unsigned long   hash = 5381;
        int 			c;
		const char      *str = s.c_str();

        while ((c = *str++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash;
    }   
};

struct str_equal{
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        return lhs == rhs;
    }   
};

} //zdcp
#endif // HASH_H_
//vim: ts=4 sw=4 cindent ai et
