idlc_generate(TARGET DeviceInfo_lib FILES "DeviceInfo.idl")
idlc_generate(TARGET DataInfo_lib FILES "DataInfo.idl")

add_library(RmtIdl_lib)

target_link_libraries(RmtIdl_lib DeviceInfo_lib DataInfo_lib CycloneDDS::ddsc)