find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file("${CMAKE_SOURCE_DIR}/server/version.h.in" "${CMAKE_BINARY_DIR}/server/version.h")
include_directories("${CMAKE_BINARY_DIR}/server") # include version.h
include_directories("${CMAKE_SOURCE_DIR}/server") # include rmt_server.h
include_directories("${CMAKE_SOURCE_DIR}/common") # include common/*.h
set(CMAKE_SWIG_FLAGS "")

if (CMAKE_VERSION VERSION_LESS "3.8")
swig_add_module(rmt_py_wrapper python python_conversion.i ${CMAKE_SOURCE_DIR}/server/rmt_server.c)
else ()
swig_add_library(rmt_py_wrapper LANGUAGE python SOURCES python_conversion.i ${CMAKE_SOURCE_DIR}/server/rmt_server.c)
endif ()    
# link to librmt_server_shared.so
swig_link_libraries(rmt_py_wrapper ${PYTHON_LIBRARIES} ${CMAKE_BINARY_DIR}/server/librmt_server_shared.so)

# copy python_example.py to 'build/swig/'
add_custom_target(swig_link_target ALL COMMAND ${CMAKE_COMMAND} -E 
                  copy "${CMAKE_SOURCE_DIR}/swig/python_example.py" "${CMAKE_BINARY_DIR}/swig/python_example.py")

# install
install(
  TARGETS ${SWIG_MODULE_rmt_py_wrapper_REAL_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${PYTHON3_PACKAGES_PATH}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/rmt_py_wrapper.py
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${PYTHON3_PACKAGES_PATH}
)