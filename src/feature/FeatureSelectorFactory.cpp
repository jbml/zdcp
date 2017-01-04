#include "FeatureSelectorFactory.h"
namespace zdcp {

#if 0
// reference for this sophisicated syntax : 
// http://gcc.gnu.org/ml/gcc-bugs/2006-11/msg01338.html

     template<>
     FeatureSelectorFactory *
     Factory<FeatureSelectorFactory, FeatureSelector>::m_instance = NULL;

     template<>
     struct FeatureSelectorFactory::InstanceTable
     Factory<FeatureSelectorFactory, FeatureSelector>::m_instanceTable[] = 
     {
         {"ChiSquare", NULL},
         {"MI", NULL},
         {"WET", NULL},
         {NULL, NULL}
     };
#endif

} // zdcp

// vim: ts=4 sw=4 ai cindent et
