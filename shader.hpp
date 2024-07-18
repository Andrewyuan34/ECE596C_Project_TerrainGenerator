#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

//inline void checkGLError(const char* stmt, const char* fname, int line)
//inline void printShaderInfoLog(GLuint shader)
//inline void printProgramInfoLog(GLuint program)
//inline void checkShaderCompileErrors(GLuint shader)
//inline void checkProgramLinkErrors(GLuint program)
//inline std::string readShaderSource(const std::string& filePath) ***
//inline GLuint loadShader(const GLchar* shaderSource, GLenum shaderType)
//inline GLuint loadShaderFromFile(const std::string& filePath, GLenum shaderType)
//inline GLuint createShaderProgram(const GLchar* vertexSource, const GLchar* fragmentSource)***
//inline GLuint createShaderProgramFromFile(const std::string& vertexFilePath, const std::string& fragmentFilePath)
//inline void useShaderProgram(GLuint program) ***
//inline void deleteShaderProgram(GLuint program) ***

// 检查OpenGL错误
inline void checkGLError(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << err << " (" << gluErrorString(err) << "), at " << fname << ":" << line << " - for " << stmt << std::endl;
        exit(1);
    }
}

#define GL_CHECK(stmt) do { \
    stmt; \
    checkGLError(#stmt, __FILE__, __LINE__); \
} while (0)

// 打印着色器信息日志
inline void printShaderInfoLog(GLuint shader) {
    GLint infoLogLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, &infoLogLength, &infoLog[0]);
        std::cerr << "Shader InfoLog:" << std::endl << &infoLog[0] << std::endl;
    }
}

// 打印程序信息日志
inline void printProgramInfoLog(GLuint program) {
    GLint infoLogLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, infoLogLength, nullptr, &infoLog[0]);
        std::cerr << "Program InfoLog:" << std::endl << &infoLog[0] << std::endl;
    }
}

// 检查着色器编译错误
inline void checkShaderCompileErrors(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        printShaderInfoLog(shader);
        std::cerr << "| ERROR::SHADER_COMPILATION_ERROR" << std::endl;
    }
}

// 检查程序链接错误
inline void checkProgramLinkErrors(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        printProgramInfoLog(program);
        std::cerr << "| ERROR::PROGRAM_LINKING_ERROR" << std::endl;
    }
}

// 从文件中读取着色器源码
inline std::string readShaderSource(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    std::stringstream shaderStream;

    if (!shaderFile.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filePath << std::endl;
        return "";
    }

    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

// 加载并编译着色器
inline GLuint loadShader(const GLchar* shaderSource, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    GL_CHECK(glShaderSource(shader, 1, &shaderSource, NULL));
    GL_CHECK(glCompileShader(shader));
    checkShaderCompileErrors(shader);
    return shader;
}

// 从文件加载着色器并编译
inline GLuint loadShaderFromFile(const std::string& filePath, GLenum shaderType) {
    std::string shaderSource = readShaderSource(filePath);
    if (shaderSource.empty()) {
        return 0;
    }
    return loadShader(shaderSource.c_str(), shaderType);
}

// 创建着色器程序并链接
inline GLuint createShaderProgram(const GLchar* vertexSource, const GLchar* fragmentSource) {
    GLuint vertexShader = loadShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    GL_CHECK(glAttachShader(program, vertexShader));
    GL_CHECK(glAttachShader(program, fragmentShader));
    GL_CHECK(glLinkProgram(program));
    checkProgramLinkErrors(program);

    GL_CHECK(glDeleteShader(vertexShader));
    GL_CHECK(glDeleteShader(fragmentShader));

    return program;
}

// 从文件创建着色器程序并链接
inline GLuint createShaderProgramFromFile(const std::string& vertexFilePath, const std::string& fragmentFilePath) {
    GLuint vertexShader = loadShaderFromFile(vertexFilePath, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShaderFromFile(fragmentFilePath, GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }

    GLuint program = glCreateProgram();
    GL_CHECK(glAttachShader(program, vertexShader));
    GL_CHECK(glAttachShader(program, fragmentShader));
    GL_CHECK(glLinkProgram(program));
    checkProgramLinkErrors(program);

    GL_CHECK(glDeleteShader(vertexShader));
    GL_CHECK(glDeleteShader(fragmentShader));

    return program;
}

// 使用着色器程序
inline void useShaderProgram(GLuint program) {
    GL_CHECK(glUseProgram(program));
}

// 删除着色器程序
inline void deleteShaderProgram(GLuint program) {
    GL_CHECK(glDeleteProgram(program));
}

#endif // SHADER_HPP

