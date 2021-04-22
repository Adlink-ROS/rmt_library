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
%array_functions(data_info, data_info_array)
%array_class(device_info, device_info_list);
%array_class(data_info, data_info_list);
%array_class(unsigned long, ulong_array);

// Using %newobject to release memory automatically
%newobject rmt_server_create_device_list;
device_info *rmt_server_create_device_list(int *num);
%newobject rmt_server_get_info;
data_info* rmt_server_get_info(unsigned long *id_list, int id_num, char *key_list, int *info_num);
%newobject rmt_server_set_info;
data_info* rmt_server_set_info(data_info *dev_list, int dev_num, int *info_num);

