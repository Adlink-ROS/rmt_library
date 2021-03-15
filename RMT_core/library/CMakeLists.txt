# Define the version
configure_file("config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Build library rmt_lib
add_library(rmt_lib rmt_lib.c)