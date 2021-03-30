# Since idlc_generate uses set_source_files_properties and it can't be seen globally, 
# we need to set_source_files_properties by ourselves.
# This file is only necessary to cmake version under 3.20
# https://cmake.org/cmake/help/latest/prop_sf/GENERATED.html#prop_sf:GENERATED

function(add_generated _target)
    set(_dir "${CMAKE_BINARY_DIR}/idl")
    set(_source "${_dir}/${_target}.c")
    set(_header "${_dir}/${_target}.h")
    set_source_files_properties(
        ${_source} ${_header} PROPERTIES GENERATED TRUE)
endfunction()

add_generated("DeviceInfo")
add_generated("DataInfo")