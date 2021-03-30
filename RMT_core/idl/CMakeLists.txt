idlc_generate(DeviceInfo_lib "DeviceInfo.idl")
idlc_generate(DataInfo_lib "DataInfo.idl")

add_library(RmtIdl_lib)

target_link_libraries(RmtIdl_lib DeviceInfo_lib DataInfo_lib CycloneDDS::ddsc)