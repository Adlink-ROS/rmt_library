# Define the version
configure_file("version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include(../idl/add_generated.cmake)

# Include common function
include_directories(${PROJECT_SOURCE_DIR}/common)

# Build library rmt_agent
add_library(rmt_agent rmt_agent.c devinfo_agent.c datainfo_agent.c ${PROJECT_SOURCE_DIR}/common/network.c)
target_link_libraries(rmt_agent PRIVATE CycloneDDS::ddsc RmtIdl_lib)

# Example (debug used)
add_executable(agent_example agent_example.c)
target_link_libraries(agent_example PRIVATE rmt_agent)