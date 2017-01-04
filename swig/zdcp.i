%module zdcp
%{
    /* Includes the header in the wrapper code */
    #include "zdcp.h"
%}

/* Parse the header file to generate wrappers */
%include "zdcp.h"

