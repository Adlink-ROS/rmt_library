# Define the version
configure_file("config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include(../idl/add_generated.cmake)

# Build library rmt_server
add_library(rmt_server rmt_server.c)

# Example (debug used)
add_executable(server_example server_example.c)
target_link_libraries(server_example PRIVATE rmt_server CycloneDDS::ddsc RmtIdl_lib)