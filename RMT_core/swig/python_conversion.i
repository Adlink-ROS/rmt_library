%module rmt_py_wrapper
%{
// Include the header in the wrapper code
#include "rmt_server.h"
%}
 
// Parse the header file to generate wrappers
%include "rmt_server.h"

// for C pointers, see http://www.swig.org/Doc4.0/SWIGDocumentation.html#Library_nn4
%include "cpointer.i"
%pointer_functions(int, intptr);

// for unbounded C arrays
%include "carrays.i"
%array_class(device_info, device_info_list);

// Using %newobject to release memory automatically
%newobject rmt_server_create_device_list;
device_info *rmt_server_create_device_list(int *num);
