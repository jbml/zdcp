#include "DocParserFactory.h"

namespace zdcp {

#if 0
// reference for this sophisicated syntax : 
// http://gcc.gnu.org/ml/gcc-bugs/2006-11/msg01338.html

     template<>
     DocParserFactory *Factory<DocParserFactory, DocParser>::m_instance = NULL;

     template<>
     struct DocParserFactory::InstanceTable
     Factory<DocParserFactory, DocParser>::m_instanceTable[] = 
     {
         {"Dcp", NULL},
         {NULL, NULL}
     };
#endif

} // zdcp

// vim: ts=4 sw=4 ai cindent et
