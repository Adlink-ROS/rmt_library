# minimum cmake version
cmake_minimum_required(VERSION 3.10)

# project name, version
project(RMT_library VERSION 0.9.3)

# Build agent or not
option(BUILD_AGENT "Build agent" ON)

# Include external project
execute_process(COMMAND git submodule update --init --recursive)
add_subdirectory(external/cyclonedds EXCLUDE_FROM_ALL)
install(
  FILES
    ${CMAKE_BINARY_DIR}/lib/libddsc.so
    ${CMAKE_BINARY_DIR}/lib/libddsc.so.0
    ${CMAKE_BINARY_DIR}/lib/libddsc.so.0.8.0
  DESTINATION
    lib
)

# Build IDL
add_subdirectory(idl)

# Build RMT server library
add_subdirectory(server)

# Build RMT Agent library
add_subdirectory(agent)

# Build docs by Doxygen. The output xml files are also needed by SWIG
find_package(Doxygen)
if (DOXYGEN_FOUND)
  set(DOXYGEN_PROJECT_NAME "RMT Library")
  set(DOXYGEN_PROJECT_NUMBER ${PROJECT_VERSION})
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_GENERATE_XML YES) # used by SWIG and pydoc
  set(DOXYGEN_GENERATE_TREEVIEW YES)
  doxygen_add_docs(
    docs
    ${PROJECT_SOURCE_DIR}/server/rmt_server.h ${PROJECT_SOURCE_DIR}/agent/rmt_agent.h
    COMMENT "Generate doxygen docs"
  )
else (DOXYGEN_FOUND)
  message("\n*** ERROR: Doxygen not found! Try 'sudo apt install doxygen' ***\n")
endif (DOXYGEN_FOUND)

# Use SWIG to generate python lib
add_subdirectory(swig)

# Build agent example
if(BUILD_AGENT)
  add_subdirectory(examples/RMT_example/agent EXCLUDE_FROM_ALL)
  # I ignore all target in subdirectory and then only pick the necessary target.
  # If I use all target, then the cpack will include the subdirectory.
  add_custom_target(build_agent ALL DEPENDS agent_example agent_link_target nb2_task_link_target docs)
endif()

# Add uncrustify tools
file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/agent/*.h
                    ${CMAKE_SOURCE_DIR}/agent/*.c
                    ${CMAKE_SOURCE_DIR}/common/*.h
                    ${CMAKE_SOURCE_DIR}/common/*.c
                    ${CMAKE_SOURCE_DIR}/server/*.h
                    ${CMAKE_SOURCE_DIR}/server/*.c
                    ${CMAKE_SOURCE_DIR}/test/*.h
                    ${CMAKE_SOURCE_DIR}/test/*.c)

set(MYCFG "linux_c.cfg")
add_custom_target(sh_target ALL
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/tools/uncrustify.sh" "${CMAKE_BINARY_DIR}/uncrustify.sh"
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/tools/${MYCFG}" "${CMAKE_BINARY_DIR}/${MYCFG}")
# Add test
add_subdirectory(test)
enable_testing()
add_test("RMT_library_API_test" test/rmt_test)
add_test("lint_test" ./uncrustify.sh ${SRC_FILES})

# Package
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_NAME "rmt_library")
set(CPACK_SET_DESTDIR true)
set(CPACK_INSTALL_PREFIX "/usr/")
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_SOURCE_DIR}/build)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ChenYing Kuo")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "RMT Library Packages")
execute_process(COMMAND dpkg --print-architecture OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
include(CPack)
