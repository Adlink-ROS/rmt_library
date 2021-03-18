add_executable(rmt_engine engine.c)
include(../idl/add_generated.cmake)

target_link_libraries(rmt_engine PRIVATE CycloneDDS::ddsc RmtIdl_lib)