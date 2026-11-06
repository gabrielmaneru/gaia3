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
target_compile_definitions(sandbox PRIVATE GAIA3_ASSETS_PATH="${CMAKE_SOURCE_DIR}/gaia3/assets/")

# Link against the core engine
target_link_libraries(sandbox PRIVATE gaia3)