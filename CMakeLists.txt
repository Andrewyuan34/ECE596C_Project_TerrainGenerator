cmake_minimum_required(VERSION 3.1)
project(PerlinNoiseOpenGL)

# 查找所需的 OpenGL 和 GLUT 库
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(GLM_INCLUDE_DIR "include/glm")

# 列出源文件
set(SOURCE_FILES
    main4.cpp
    PerlinNoise.cpp
    PerlinNoise.hpp
    #camera.cpp
    #camera.hpp
)

# 创建可执行文件
add_executable(perlin_noise_visualization ${SOURCE_FILES})

# 包含 OpenGL 和 GLUT 库
include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${GLM_INCLUDE_DIR})

# 链接 OpenGL 和 GLUT 库
target_link_libraries(perlin_noise_visualization ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} -lGL -lGLU -lglut)


