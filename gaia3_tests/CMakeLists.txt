# Create gaia3_tests executable
add_executable(gaia3_tests
    src/main.cpp
    src/components/components_tests.cpp
    src/core/log_tests.cpp
    src/core/logPolicy_tests.cpp
    src/core/serializationTypes_tests.cpp
    src/core/session_tests.cpp
    src/gfx/meshObj_tests.cpp
    src/math/mathHelpers_tests.cpp
    src/math/matrix_tests.cpp
    src/math/quaternion_tests.cpp
    src/math/transform_tests.cpp
    src/math/vector_tests.cpp
    src/scene/authoring_tests.cpp
    src/scene/componentFactory_tests.cpp
    src/scene/editor_tests.cpp
    src/scene/entity_tests.cpp
    src/scene/level_tests.cpp
    src/scene/testComponents.cpp
    src/system/resource_tests.cpp
    src/system/resourceManager_tests.cpp
    src/system/resourceRef_tests.cpp
    src/system/rtti_tests.cpp
)

# Apply C++ 20 Standard
target_compile_features(gaia3_tests PRIVATE cxx_std_20)

# Include directories from the core engine (gaia3)
target_include_directories(gaia3_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../gaia3/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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