add_executable(rmt_engine engine.c)
target_link_libraries(rmt_engine PRIVATE CycloneDDS::ddsc IdlData_lib)