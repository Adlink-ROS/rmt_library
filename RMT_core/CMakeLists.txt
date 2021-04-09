# minimum cmake version
cmake_minimum_required(VERSION 3.10)

# project name, version
project(RMT_core VERSION 0.9.0)

# Include external project
# RMT_TODO: upgrade CycloneDDS to 0.8.0
execute_process(COMMAND git submodule update --init)
add_subdirectory(external/cyclonedds)

# Build IDL
add_subdirectory(idl)

# Build RMT server
add_subdirectory(server)

# Build RMT Agent
add_subdirectory(agent)

# Use SWIG to generate python lib
add_subdirectory(swig)

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