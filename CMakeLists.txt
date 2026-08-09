cmake_minimum_required(VERSION 3.28)
project(TerrainGenerator VERSION 2.0.0 LANGUAGES C CXX)

# ---- Language configuration -------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT MSVC)
    message(FATAL_ERROR "TerrainGenerator currently supports MSVC on Windows only.")
endif()

# ---- Dependencies (FetchContent, everything stays inside the build tree) ----
include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    URL "https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip")

FetchContent_Declare(glm
    URL "https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip")

FetchContent_Declare(CLI11
    URL "https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.4.2.zip")

FetchContent_Declare(glad
    URL "https://github.com/Dav1dde/glad/archive/refs/heads/glad2.zip")

FetchContent_MakeAvailable(glfw glm CLI11 glad)

# glad2 keeps its CMake integration in the cmake/ subdirectory.
add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)

# Generate an OpenGL 3.3 Core loader (requires a Python interpreter).
glad_add_library(glad_gl_core_33 STATIC API gl:core=3.3)

# ---- Target -----------------------------------------------------------------
add_executable(terrain_generator
    src/main.cpp
    src/cli.cpp
    src/perlin_noise.cpp
    src/terrain_mesh.cpp
    src/camera.cpp
    src/application.cpp
)

target_include_directories(terrain_generator PRIVATE src)

target_link_libraries(terrain_generator PRIVATE
    glad_gl_core_33
    glfw
    glm::glm
    CLI11::CLI11
)

target_compile_options(terrain_generator PRIVATE /W4 /permissive- /utf-8 /EHsc)

# Runtime assets (shaders / textures) are copied next to the executable.
add_custom_command(TARGET terrain_generator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shader" "$<TARGET_FILE_DIR:terrain_generator>/shader"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/texture" "$<TARGET_FILE_DIR:terrain_generator>/texture"
)
