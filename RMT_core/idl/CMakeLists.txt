idlc_generate(DeviceInfo_lib "DeviceInfo.idl")
add_library(RmtIdl_lib)

target_link_libraries(RmtIdl_lib DeviceInfo_lib CycloneDDS::ddsc)