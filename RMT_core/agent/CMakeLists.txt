include(../idl/add_generated.cmake)

add_executable(agent_example agent_example.c)

target_link_libraries(agent_example PRIVATE CycloneDDS::ddsc RmtIdl_lib)