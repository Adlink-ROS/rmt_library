idlc_generate(DeviceInfo_lib "DeviceInfo.idl")
add_library(IdlData_lib)

target_link_libraries(IdlData_lib DeviceInfo_lib CycloneDDS::ddsc)