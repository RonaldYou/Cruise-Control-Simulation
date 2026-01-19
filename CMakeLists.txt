cmake_minimum_required(VERSION 3.14)  # 3.14 required for FetchContent_MakeAvailable

# Project name
project(CruiseControlSimulator)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# =============================================================================
# FetchContent: Downloads and builds external dependencies at configure time
# This avoids requiring users to manually install libraries
# =============================================================================
include(FetchContent)

# -----------------------------------------------------------------------------
# GLFW: Cross-platform library for creating windows, OpenGL contexts, and
#       handling input (keyboard, mouse, joystick). We disable building
#       docs/tests/examples to speed up compilation.
# -----------------------------------------------------------------------------
FetchContent_Declare(
    glfw                                    # Name we'll use to reference this dependency
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8                    # Stable release version
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)     # Don't build documentation
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)    # Don't build test programs
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # Don't build example programs

# -----------------------------------------------------------------------------
# GLM: Header-only math library for graphics (vectors, matrices, quaternions).
#      Provides GLSL-like syntax in C++ (vec3, mat4, etc.)
# -----------------------------------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1                    # Newer version with updated CMake
)

# Download and make both dependencies available
FetchContent_MakeAvailable(glfw glm)

# -----------------------------------------------------------------------------
# GLAD: OpenGL function loader. OpenGL functions aren't directly available -
#       they must be loaded at runtime. GLAD generates code to do this.
#       We include it as source since it's generated for specific GL version.
# -----------------------------------------------------------------------------
add_library(glad STATIC
    external/glad/src/glad.c                # The loader implementation
)
target_include_directories(glad PUBLIC
    external/glad/include                   # Headers: glad/glad.h, KHR/khrplatform.h
)

# Add include directory for headers
include_directories(include)

# Gather source files
set(SOURCES
    src/main.cpp
    src/Vehicle.cpp
    src/Car.cpp
    src/PIDController.cpp
    src/Simulator.cpp
    src/Terrain.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/Renderer.cpp
)

# Build executable
add_executable(cruise_sim ${SOURCES})

# -----------------------------------------------------------------------------
# Link libraries to our executable:
# - glfw: Window/context/input management
# - glad: OpenGL function loading
# - glm: Math operations (header-only, but linking ensures include paths work)
# -----------------------------------------------------------------------------
target_link_libraries(cruise_sim PRIVATE
    glfw        # Links libglfw, provides window creation and input
    glad        # Links our glad library for GL function loading
    glm         # Header-only but CMake handles include paths
)