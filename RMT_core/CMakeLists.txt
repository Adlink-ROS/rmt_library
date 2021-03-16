# minimum cmake version
cmake_minimum_required(VERSION 3.10)

# project name, version
project(RMT_core VERSION 0.9.0)

# Include external project
#execute_process(COMMAND git submodule update --init)
#add_subdirectory(external/cyclonedds)

# Build RMT library
add_subdirectory(library)

# Add test
add_subdirectory(test)
enable_testing()
add_test("RMT library API test" test/rmt_test)