cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(Terrain_Generator)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL, GLEW, GLUT libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Include Boost program_options
find_package(Boost REQUIRED COMPONENTS program_options)

# Add executable target
add_executable(terrain_generator
    src/main.cpp
    src/command_line_parser.cpp
    src/camera.cpp
    src/lighting.cpp
    src/TerrainGenerate.cpp
)

# Include directories
target_include_directories(terrain_generator PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    src
)

# Link libraries
target_link_libraries(terrain_generator
    ${Boost_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
)

# Install the terrain_generator executable to the bin directory.
install(TARGETS terrain_generator DESTINATION bin)

# Install the demo script to the bin directory.
install(PROGRAMS demo DESTINATION bin)