include(../idl/add_generated.cmake)

# Build rmt_test
add_executable(rmt_test run_test.c testcases_basic.c)

target_include_directories(rmt_test PRIVATE ${PROJECT_SOURCE_DIR}/server)
target_include_directories(rmt_test PRIVATE ${PROJECT_SOURCE_DIR}/agent)
target_link_libraries(rmt_test PRIVATE cunit)
target_link_libraries(rmt_test PRIVATE rmt_server)
target_link_libraries(rmt_test PRIVATE rmt_agent)