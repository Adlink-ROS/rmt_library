add_executable(rmt_engine engine.c)
include(../comm_idl/add_generated.cmake)

target_link_libraries(rmt_engine PRIVATE CycloneDDS::ddsc IdlData_lib)