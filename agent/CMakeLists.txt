# Define the version
configure_file("version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include(../idl/add_generated.cmake)

# Include common function
include_directories(${PROJECT_SOURCE_DIR}/common)

# Source Code List
set(SOURCE_CODE rmt_agent.c
                devinfo_agent.c
                datainfo_agent.c
                ${PROJECT_SOURCE_DIR}/common/network.c
                ${PROJECT_SOURCE_DIR}/common/dds_transport.c
                ${PROJECT_SOURCE_DIR}/common/logger.c
                ${PROJECT_SOURCE_DIR}/common/config.c
   )

# Build library rmt_agent
add_library(rmt_agent SHARED ${SOURCE_CODE})

set(NEURON_LICENSE_MANAGER_PATH /opt/neuron-license-manager)
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  # Need these library to do license verification
  set(TARGET_BUILD_LICENSE
      "-Wl,--whole-archive -L${NEURON_LICENSE_MANAGER_PATH}/lib -lproduct_verifier -lLicenseInjector-NeuronSDK -llicense++_static -los -lCryptoHelper -lbase -lini -lcrypto -lstdc++ -Wl,--no-whole-archive")
else()
  # We need product_verifier to check the model name
  set(TARGET_BUILD_LICENSE
      "-Wl,--whole-archive -L${NEURON_LICENSE_MANAGER_PATH}/lib -lproduct_verifier -Wl,--no-whole-archive")
endif()
set_target_properties(rmt_agent PROPERTIES LINK_FLAGS ${TARGET_BUILD_LICENSE})
target_link_libraries(rmt_agent PRIVATE CycloneDDS::ddsc RmtIdl_lib)

# Make sure warning as error
target_compile_options(rmt_agent PRIVATE -Wall -Wextra -pedantic -Werror)

# Make header public
set_target_properties(rmt_agent PROPERTIES PUBLIC_HEADER "rmt_agent.h")

# Install
install(TARGETS rmt_agent
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)