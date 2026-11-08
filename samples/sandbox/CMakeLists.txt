# Create sandbox executable
add_executable(sandbox
    src/sandbox.cpp
)

# Apply C++ 20 Standard
target_compile_features(sandbox PRIVATE cxx_std_20)

# Include directories from the core engine (gaia3)
target_include_directories(sandbox PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../gaia3/src
)

# Note: Absolute so assets are found from any working directory
target_compile_definitions(sandbox PRIVATE
    GAIA3_ASSETS_PATH="${CMAKE_SOURCE_DIR}/gaia3/assets/"
    SANDBOX_ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)

# Link against the core engine
# Note: Whole archive keeps every engine file, so every resource type and component registers itself
target_link_libraries(sandbox PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,gaia3>")

# Note: These open a real window for a few frames, so they need a GPU and run one at a time
add_test(NAME sandbox_docked_editor COMMAND sandbox -frames 30)
add_test(NAME sandbox_floating_editor COMMAND sandbox -no_dock -frames 30)
add_test(NAME sandbox_no_editor COMMAND sandbox -no_editor -frames 30)
set_tests_properties(sandbox_docked_editor sandbox_floating_editor sandbox_no_editor PROPERTIES RUN_SERIAL TRUE)