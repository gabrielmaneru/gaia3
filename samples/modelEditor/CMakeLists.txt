# Create modelEditor executable
add_executable(modelEditor
    src/main.cpp
    src/editableModel.cpp
    src/modelEditor.cpp
    src/modelOperations.cpp
    src/components/previewModelComponent.cpp
)

# Apply C++ 20 Standard
target_compile_features(modelEditor PRIVATE cxx_std_20)

# Include directories from the core engine (gaia3) and the editor's own sources
target_include_directories(modelEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../gaia3/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Note: Absolute so assets are found from any working directory. The editor works on a project's assets, the
# sandbox's by default, and can be pointed at another with -project
target_compile_definitions(modelEditor PRIVATE
    GAIA3_ASSETS_PATH="${CMAKE_SOURCE_DIR}/gaia3/assets/"
    DEFAULT_PROJECT_PATH="${CMAKE_SOURCE_DIR}/samples/sandbox/assets/"
)

# Link against the core engine
# Note: Whole archive keeps every engine file, so every resource type and component registers itself
target_link_libraries(modelEditor PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,gaia3>")

# Note: Opens a real window for a few frames, so it needs a GPU
add_test(NAME model_editor_starts COMMAND modelEditor -frames 30)
set_tests_properties(model_editor_starts PROPERTIES RUN_SERIAL TRUE)

# Note: The editor's own tests, for the code that lives in the editor rather than the engine
if(GAIA3_BUILD_TESTS)
    add_executable(modelEditor_tests
        tests/modelOperations_tests.cpp
        src/modelOperations.cpp
    )

    target_compile_features(modelEditor_tests PRIVATE cxx_std_20)

    target_include_directories(modelEditor_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../gaia3/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_compile_definitions(modelEditor_tests PRIVATE
        GAIA3_ASSETS_PATH="${CMAKE_SOURCE_DIR}/gaia3/assets/"
    )

    find_package(GTest REQUIRED)

    target_link_libraries(modelEditor_tests PRIVATE
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,gaia3>"
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(modelEditor_tests)
endif()
