# Define the version
configure_file("config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include(../idl/add_generated.cmake)

# Build library rmt_lib
add_library(rmt_lib rmt_lib.c)

# Example (debug used)
add_executable(rmt_example example.c)
target_link_libraries(rmt_example PRIVATE rmt_lib CycloneDDS::ddsc RmtIdl_lib)