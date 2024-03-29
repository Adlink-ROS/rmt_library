find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

configure_file("${CMAKE_SOURCE_DIR}/server/version.h.in" "${CMAKE_BINARY_DIR}/server/version.h")
include_directories("${CMAKE_BINARY_DIR}/server") # include version.h
include_directories("${CMAKE_SOURCE_DIR}/server") # include rmt_server.h
include_directories("${CMAKE_SOURCE_DIR}/common") # include common/*.h

if (BUILD_RMT_DOCS)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    set(CMAKE_SWIG_FLAGS -DDOXYGEN=${DOXYGEN_FOUND})
  endif (DOXYGEN_FOUND)
endif (BUILD_RMT_DOCS)

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
execute_process(COMMAND python3 -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(), end = '')" OUTPUT_VARIABLE PYTHON3_PATH)
install(
  TARGETS ${SWIG_MODULE_rmt_py_wrapper_REAL_NAME}
  DESTINATION ${PYTHON3_PATH}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/rmt_py_wrapper.py
  DESTINATION ${PYTHON3_PATH}
)

if (BUILD_RMT_DOCS)
  if (DOXYGEN_FOUND)
    set(DOCFILES rmt__agent rmt__server)
    foreach(_file ${DOCFILES})
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}_doc.i
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/doxy2swig.py
          ${CMAKE_BINARY_DIR}/docs/xml/${_file}_8h.xml
          ${CMAKE_CURRENT_BINARY_DIR}/${_file}_doc.i
        DEPENDS ${CMAKE_BINARY_DIR}/docs/xml/${_file}_8h.xml
      )
      add_custom_target(${_file}doc_i DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_file}_doc.i)
      add_dependencies(${SWIG_MODULE_rmt_py_wrapper_REAL_NAME} ${_file}doc_i)
    endforeach()

    # build --target pydoc
    add_custom_target(pydoc
      pydoc3 -w rmt_py_wrapper python_example
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating Python API documentation with pydoc" VERBATIM
    )
  else (DOXYGEN_FOUND)
    message("\n*** ERROR: Doxygen not found! Try 'sudo apt install doxygen' ***\n")
  endif (DOXYGEN_FOUND)
endif (BUILD_RMT_DOCS)