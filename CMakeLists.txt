cmake_minimum_required(VERSION 3.28)
project(TerrainGenerator VERSION 2.0.0 LANGUAGES C CXX)

# ---- Language configuration -------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Dependencies (FetchContent, everything stays inside the build tree) ----
include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    URL "https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip"
    URL_HASH SHA256=b5ec004b2712fd08e8861dc271428f048775200a2df719ccf575143ba749a3e9
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_Declare(glm
    URL "https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip"
    URL_HASH SHA256=09c5716296787e1f7fcb87b1cbdbf26814ec1288ed6259ccd30d5d9795809fa5
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_Declare(CLI11
    URL "https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.4.2.zip"
    URL_HASH SHA256=43e650d5e1a3acaaf419d1e61a81f77b408d0696f472be0599ddf877d40984b0
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_Declare(glad
    # Pinned instead of following the mutable glad2 branch.
    URL "https://github.com/Dav1dde/glad/archive/e3c14f906127dea49ead20ff4d5878294d08b7ab.zip"
    URL_HASH SHA256=882cc167a31421b910e3846b40fdc9ca2469e5a9054bab980a94435151b1cf79
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_Declare(imgui
    URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.9.zip"
    URL_HASH SHA256=3fe9e93066e37c630cbbada821a45812cd86df0f21a476cf74c925e1b649878d
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_MakeAvailable(glfw glm CLI11 glad imgui)

# glad2 keeps its CMake integration in the cmake/ subdirectory.
add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)

# Generate an OpenGL 3.3 Core loader from glad's bundled registry snapshots.
# REPRODUCIBLE prevents network access to mutable Khronos XML files.
find_package(Python 3.8 COMPONENTS Interpreter REQUIRED)
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c "import jinja2"
    RESULT_VARIABLE TERRAIN_JINJA2_STATUS
    OUTPUT_QUIET ERROR_QUIET)
if (NOT TERRAIN_JINJA2_STATUS EQUAL 0)
    message(FATAL_ERROR
        "glad requires Jinja2. Install requirements-build.txt into the active Python environment.")
endif()
glad_add_library(glad_gl_core_33 STATIC REPRODUCIBLE API gl:core=3.3)

# Dear ImGui intentionally ships without a CMake target. Keep the target local
# and compile only the core plus the GLFW/OpenGL 3 backends used by this app.
add_library(terrain_imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(terrain_imgui PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
)
target_link_libraries(terrain_imgui PRIVATE glfw)

# ---- Target -----------------------------------------------------------------
set(TERRAIN_GENERATOR_SOURCES
    src/main.cpp
    src/cli.cpp
    src/perlin_noise.cpp
    src/terrain_mesh.cpp
    src/camera.cpp
    src/frustum.cpp
    src/gl_raii.cpp
    src/application.cpp
)
file(GLOB TERRAIN_PROJECT_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")

# Ninja cannot parse localized MSVC /showIncludes output on every Windows
# locale. Explicit project-header dependencies keep incremental builds correct
# without depending on diagnostic language or code page.
if (MSVC)
    set_source_files_properties(${TERRAIN_GENERATOR_SOURCES}
        PROPERTIES OBJECT_DEPENDS "${TERRAIN_PROJECT_HEADERS}")
endif()

function(terrain_enable_warnings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8 /EHsc)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_executable(terrain_generator
    ${TERRAIN_GENERATOR_SOURCES}
    ${TERRAIN_PROJECT_HEADERS}
)

target_include_directories(terrain_generator PRIVATE src)

target_link_libraries(terrain_generator PRIVATE
    glad_gl_core_33
    glfw
    glm::glm
    CLI11::CLI11
    terrain_imgui
)

terrain_enable_warnings(terrain_generator)

# Runtime assets (shaders / textures) are synchronized on every build so an
# asset-only edit cannot leave a stale copy beside an up-to-date executable.
add_custom_target(terrain_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shader" "$<TARGET_FILE_DIR:terrain_generator>/shader"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/texture" "$<TARGET_FILE_DIR:terrain_generator>/texture"
    COMMENT "Synchronizing runtime assets"
)
add_dependencies(terrain_generator terrain_assets)

include(GNUInstallDirs)
install(TARGETS terrain_generator RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY shader texture DESTINATION "${CMAKE_INSTALL_BINDIR}")

# ---- Tests ------------------------------------------------------------------
include(CTest)
if (BUILD_TESTING)
    if (MSVC)
        set_source_files_properties(tests/core_tests.cpp
            PROPERTIES OBJECT_DEPENDS "${TERRAIN_PROJECT_HEADERS}")
    endif()
    add_executable(terrain_core_tests
        tests/core_tests.cpp
        src/cli.cpp
        src/perlin_noise.cpp
        src/terrain_mesh.cpp
        src/frustum.cpp
    )
    target_include_directories(terrain_core_tests PRIVATE src)
    target_link_libraries(terrain_core_tests PRIVATE glm::glm CLI11::CLI11)
    terrain_enable_warnings(terrain_core_tests)

    add_test(NAME terrain_core COMMAND terrain_core_tests)
    add_test(NAME terrain_smoke
        COMMAND terrain_generator --screenshot terrain-smoke.bmp)
    set_tests_properties(terrain_smoke PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:terrain_generator>")

    add_test(NAME terrain_screenshot_failure
        COMMAND terrain_generator --screenshot missing-directory/terrain-smoke.bmp)
    set_tests_properties(terrain_screenshot_failure PROPERTIES
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:terrain_generator>"
        WILL_FAIL TRUE)
endif()
