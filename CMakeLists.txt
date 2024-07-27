cmake_minimum_required(VERSION 3.21)

# Define the Base-N Library project
project(bases
        VERSION 1.0.0.0
        DESCRIPTION "Base-N Encoder/Decoder Library"
        LANGUAGES CXX)

# Tests are built by default when this is a top-level project
if(PROJECT_IS_TOP_LEVEL)
    # Option to control whether tests are built
    option(bases_BUILD_TESTS "Build Tests for the Base-N Library" ON)
else()
    # Option to control whether tests are built
    option(bases_BUILD_TESTS "Build Tests for the Base-N Library" OFF)
endif()

# Option to control ability to install the library
option(bases_INSTALL "Install the Base-N Library" ON)

add_subdirectory(dependencies)
add_subdirectory(src)

include(CTest)

if(BUILD_TESTING AND bases_BUILD_TESTS)
    add_subdirectory(test)
endif()
