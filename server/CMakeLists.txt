# Define the version
configure_file("version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include(../idl/add_generated.cmake)

# Include common function
include_directories(${PROJECT_SOURCE_DIR}/common)

# Source Code List
set(SOURCE_CODE rmt_server.c
                devinfo_server.c
                datainfo_server.c
                server_config.c
                ${PROJECT_SOURCE_DIR}/common/network.c
                ${PROJECT_SOURCE_DIR}/common/dds_transport.c
   )

# Build library rmt_server
add_library(rmt_server STATIC ${SOURCE_CODE})
target_link_libraries(rmt_server PRIVATE CycloneDDS::ddsc RmtIdl_lib pthread)

# Build library rmt_server_shared for SWIG
add_library(rmt_server_shared SHARED ${SOURCE_CODE})
target_link_libraries(rmt_server_shared PRIVATE CycloneDDS::ddsc RmtIdl_lib pthread)

# Example (debug used)
add_executable(server_example server_example.c)
target_link_libraries(server_example PRIVATE rmt_server)

# Make header public
set_target_properties(rmt_server PROPERTIES PUBLIC_HEADER "rmt_server.h")

# Install
install(TARGETS rmt_server rmt_server_shared
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)