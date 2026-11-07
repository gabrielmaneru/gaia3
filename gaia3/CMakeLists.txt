# Create Static library gaia3
add_library(gaia3 STATIC
    src/components/cameraComponent.cpp
    src/components/modelComponent.cpp
    src/core/app.cpp
    src/core/log.cpp
    src/core/session.cpp
    src/core/serializationTypes.cpp
    src/gfx/framebuffer.cpp
    src/gfx/meshObj.cpp
    src/gfx/model.cpp
    src/gfx/renderer.cpp
    src/gfx/rendererTypes.cpp
    src/math/mathHelpers.cpp
    src/math/matrix.cpp
    src/math/matrixTransforms.cpp
    src/math/quaternion.cpp
    src/math/quaternionTransforms.cpp
    src/math/transform.cpp
    src/math/vector.cpp
    src/math/vectorTransforms.cpp
    src/platform/input.cpp
    src/platform/window.cpp
    src/scene/component.cpp
    src/scene/componentFactory.cpp
    src/scene/editor.cpp
    src/scene/entity.cpp
    src/scene/inspector.cpp
    src/scene/level.cpp
    src/system/resource.cpp
    src/system/resourceManager.cpp
    src/system/rtti.cpp
)

# Apply C++ 20 Standard
target_compile_features(gaia3 PUBLIC cxx_std_20)
# Configure Precompiled Headers
target_precompile_headers(gaia3 PRIVATE src/gaia3_pch.h)

# Include Directories
target_include_directories(gaia3 PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Find vcpkg packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)

# Link against vcpkg packages
target_link_libraries(gaia3 PUBLIC
    glfw
    glad::glad
    jsoncpp_lib
    spdlog::spdlog
)

# Note: Private because ImGui is an implementation detail of the editor
target_link_libraries(gaia3 PRIVATE imgui::imgui imguizmo::imguizmo)