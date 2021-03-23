# Define the version
configure_file("config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include(../idl/add_generated.cmake)

# Build library rmt_agent
add_library(rmt_agent rmt_agent.c devinfo_agent.c)
target_link_libraries(rmt_agent PRIVATE CycloneDDS::ddsc RmtIdl_lib)

# Example (debug used)
add_executable(agent_example agent_example.c)
target_link_libraries(agent_example PRIVATE rmt_agent)