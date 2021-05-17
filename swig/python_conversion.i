%module rmt_py_wrapper
%{
// Include the header in the wrapper code
#include "rmt_server.h"
%}
 
%include "typemaps.i"
// typemap for the write buffer of rmt_server_send_file()
%typemap(in) (void *pFile, unsigned long file_len) {
    if (!PyString_Check($input)) {
        PyErr_SetString(PyExc_ValueError, "Expecting a binary string");
        SWIG_fail;
    }
    $1 = PyString_AsString($input);
    $2 = PyString_Size($input);
}

// In python, it's not required to put 'transfer_result' as a function parameter
// to get the result. Instead, python function can return multiple values with 
// different types. Therefore, we just set 'numinputs=0' to ignore this input 
// parameter and then use 'argout' to append it to return value
%typemap(in,numinputs=0) transfer_result* (transfer_result tmp) %{
    $1 = &tmp;
%}
%typemap(argout) transfer_result* (PyObject* o) %{
    // Blow away any previous result
    Py_XDECREF($result);

    // for transfer_result->result
    o = PyLong_FromLong($1->result);
    $result = SWIG_Python_AppendOutput($result, o);

    // for transfer_result->pFile and transfer_result->file_len, return as a bytearray
    o = PyByteArray_FromStringAndSize((char*) $1->pFile, $1->file_len);
    $result = SWIG_Python_AppendOutput($result, o);
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
%newobject rmt_server_set_info_with_same_value;
data_info* rmt_server_set_info_with_same_value(unsigned long *id_list, int id_num, char *value_list, int *info_num);

