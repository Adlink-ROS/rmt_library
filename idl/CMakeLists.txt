idlc_generate(TARGET DeviceInfo_lib FILES "DeviceInfo.idl")
idlc_generate(TARGET DataInfo_lib FILES "DataInfo.idl")
if(SUPPORT_ZENOH)
  idlc_generate(TARGET far_dds_bridge_msgs_lib FILES "far_dds_bridge_msgs.idl")
endif()

add_library(RmtIdl_lib)

if(SUPPORT_ZENOH)
  target_link_libraries(RmtIdl_lib DeviceInfo_lib DataInfo_lib far_dds_bridge_msgs_lib CycloneDDS::ddsc)
else()
  target_link_libraries(RmtIdl_lib DeviceInfo_lib DataInfo_lib CycloneDDS::ddsc)
endif()