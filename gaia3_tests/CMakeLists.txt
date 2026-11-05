# Create gaia3_tests executable
add_executable(gaia3_tests
    src/main.cpp
    src/math/matrix_tests.cpp
    src/math/quaternion_tests.cpp
    src/math/transform_tests.cpp
    src/math/vector_tests.cpp
)

# Apply C++ 20 Standard
target_compile_features(gaia3_tests PRIVATE cxx_std_20)

# Include directories from the core engine (gaia3)
target_include_directories(gaia3_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../gaia3/src
)

# Find vcpkg packages
find_package(GTest REQUIRED)

# Link against the core engine and vcpkg packages
target_link_libraries(gaia3_tests PRIVATE 
    gaia3 
    GTest::gtest_main
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(gaia3_tests)