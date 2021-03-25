FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/server/config.h.in" "${CMAKE_BINARY_DIR}/server/config.h")
INCLUDE_DIRECTORIES("${CMAKE_BINARY_DIR}/server") # include config.h
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/server") # include rmt_server.h
SET(CMAKE_SWIG_FLAGS "")

if (CMAKE_VERSION VERSION_LESS "3.8")
SWIG_ADD_MODULE(rmt_py_wrapper python python_conversion.i ${CMAKE_SOURCE_DIR}/server/rmt_server.c)
else ()
swig_add_library(rmt_py_wrapper LANGUAGE python SOURCES python_conversion.i ${CMAKE_SOURCE_DIR}/server/rmt_server.c)
endif ()    
# link to librmt_server_shared.so
SWIG_LINK_LIBRARIES(rmt_py_wrapper ${PYTHON_LIBRARIES} ${CMAKE_BINARY_DIR}/server/librmt_server_shared.so)

# copy python_example.py to 'build/swig/'
ADD_CUSTOM_TARGET(link_target ALL COMMAND ${CMAKE_COMMAND} -E 
                  copy "${CMAKE_SOURCE_DIR}/swig/python_example.py" "${CMAKE_BINARY_DIR}/swig/python_example.py")