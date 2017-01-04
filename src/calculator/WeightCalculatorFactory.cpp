#include "WeightCalculatorFactory.h"

namespace zdcp {

#if 0    
// reference for this sophisicated syntax : 
// http://gcc.gnu.org/ml/gcc-bugs/2006-11/msg01338.html

     template<>
     WeightCalculatorFactory *
     Factory<WeightCalculatorFactory, WeightCalculator>::m_instance = NULL;

     template<>
     struct WeightCalculatorFactory::InstanceTable
     Factory<WeightCalculatorFactory, WeightCalculator>::m_instanceTable[] = 
     {
         {"TW", NULL},
         {"TF", NULL},
         {"TFIDF", NULL},
         {NULL, NULL}
     };
#endif

} // zdcp

// vim: ts=4 sw=4 ai cindent et
