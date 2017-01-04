#include "LearnerFactory.h"
namespace zdcp {

#if 0
// reference for this sophisicated syntax : 
// http://gcc.gnu.org/ml/gcc-bugs/2006-11/msg01338.html

     template<>
     LearnerFactory *Factory<LearnerFactory, Learner>::m_instance = NULL;

     template<>
     struct LearnerFactory::InstanceTable
     Factory<LearnerFactory, Learner>::m_instanceTable[] = 
     {
         {"Svm", NULL},
         {"LogLinear", NULL},
         {NULL, NULL}
     };
#endif

} // zdcp

// vim: ts=4 sw=4 ai cindent et
