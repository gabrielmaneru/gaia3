cmake_minimum_required(VERSION 3.24)
project(gaia3 VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option flags for selective builds
option(GAIA3_BUILD_TESTS "Build gaia3 test suite" ON)
option(GAIA3_BUILD_SAMPLES "Build gaia3 sample projects" ON)

# Compile option flags
add_compile_options(-Wno-invalid-offsetof) # Silence invalid-offsetof warning

# 1. Base Engine Library
add_subdirectory(gaia3)

# 2. Test Suite
if(GAIA3_BUILD_TESTS)
    enable_testing()
    add_subdirectory(gaia3_tests)
endif()

# 3. Sample Projects
if(GAIA3_BUILD_SAMPLES)
    add_subdirectory(samples/sandbox)
endif()