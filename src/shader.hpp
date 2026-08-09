#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>

// Check for OpenGL errors
inline void checkGLError(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << err << " (" << gluErrorString(err) << "), at " << fname << ":" << line << " - for " << stmt << '\n';
        exit(1);
    }
}

#define GL_CHECK(stmt) do { \
    stmt; \
    checkGLError(#stmt, __FILE__, __LINE__); \
} while (0)

// Print shader information log
inline void printShaderInfoLog(GLuint shader) {
    GLint infoLogLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, &infoLogLength, &infoLog[0]);
        std::cerr << "Shader InfoLog:" << '\n' << &infoLog[0] << '\n';
    }
}

// Print program information log
inline void checkShaderCompileErrors(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::string log(logLength, '\0');
            glGetShaderInfoLog(shader, logLength, NULL, &log[0]);
            std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << log << '\n';
        }
        glDeleteShader(shader); // Delete shader to free up resources
    }
}

// Check for program linking errors
inline void checkProgramLinkErrors(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        printShaderInfoLog(program);
        std::cerr << "| ERROR::PROGRAM_LINKING_ERROR" << '\n';
    }
}

// Read shader source from file
inline std::string readShaderSource(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    std::stringstream shaderStream;
    
    if (!shaderFile.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filePath << '\n';
        return "";
    }

    shaderStream << shaderFile.rdbuf();

    if (shaderFile.fail()) {
        std::cerr << "ERROR::SHADER::FAILED_TO_READ_DATA: " << filePath << '\n';
        shaderFile.close();
        return "";
    }

    shaderFile.close();
    return shaderStream.str();
}

// Load shader and compile
inline GLuint loadShader(const GLchar* shaderSource, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    if (shader == 0) {
        std::cerr << "ERROR::SHADER::CREATION_FAILED: Could not create shader." << '\n';
        return 0;
    }

    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    
    checkShaderCompileErrors(shader);

    return shader;
}

// Load shader from file and compile
inline GLuint loadShaderFromFile(const std::string& filePath, GLenum shaderType) {
    std::string shaderSource = readShaderSource(filePath);
    if (shaderSource.empty()) {
        return 0;
    }
    return loadShader(shaderSource.c_str(), shaderType);
}

// Load texture from file
inline GLuint loadTexture(const std::filesystem::path& imagepath) {
    std::cout << "Reading image " << imagepath << '\n';

    FILE* file = fopen(imagepath.string().c_str(), "rb");
    if (!file) {
        std::cerr << "Image could not be opened: " << imagepath << '\n';
        return 0;
    }

    // Load the BMP file header
    unsigned char header[54];
    if (fread(header, 1, 54, file) != 54) {
        std::cerr << "Not a correct BMP file: " << imagepath << '\n';
        fclose(file);
        return 0;
    }

    if (header[0] != 'B' || header[1] != 'M') {
        std::cerr << "Not a correct BMP file: " << imagepath << '\n';
        fclose(file);
        return 0;
    }

    // Get image information
    unsigned int dataPos = *(int*)&(header[0x0A]);
    unsigned int imageSize = *(int*)&(header[0x22]);
    unsigned int width = *(int*)&(header[0x12]);
    unsigned int height = *(int*)&(header[0x16]);

    // Correct image size if it is not present in the header
    if (imageSize == 0) imageSize = width * height * 3;
    if (dataPos == 0) dataPos = 54;

    // Set the file position to the beginning of the image data
    std::vector<unsigned char> data(imageSize);

    // Load the image data
    if (fread(data.data(), 1, imageSize, file) != imageSize) {
        std::cerr << "Error reading image data: " << imagepath << '\n';
        fclose(file);
        return 0;
    }

    // Close the file
    fclose(file);

    // Create OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 设置为使用 Mipmap 的线性过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load and generate the texture
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_BGR, GL_UNSIGNED_BYTE, data.data());

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << error << '\n';
        glDeleteTextures(1, &textureID); // Delete the texture to free up resources
        return 0;
    }

    std::cout << "Successfully loaded " << imagepath << '\n';

    return textureID;
}


// 创建着色器程序并链接
inline GLuint createShaderProgram(const GLchar* vertexSource, const GLchar* fragmentSource) {
    GLuint vertexShader = loadShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    if (program == 0) {
        std::cerr << "ERROR::SHADER::PROGRAM::CREATION_FAILED: Could not create shader program." << '\n';
        return 0;
    }
    
    GL_CHECK(glAttachShader(program, vertexShader));
    GL_CHECK(glAttachShader(program, fragmentShader));
    GL_CHECK(glLinkProgram(program));

    // Check for linking errors
    checkProgramLinkErrors(program);

    // Delete the shaders as they're linked into our program now and no longer necessery
    GL_CHECK(glDeleteShader(vertexShader));
    GL_CHECK(glDeleteShader(fragmentShader));

    return program;
}

// 从文件创建着色器程序并链接
inline GLuint createShaderProgramFromFile(const std::string& vertexFilePath, const std::string& fragmentFilePath) {
    GLuint vertexShader = loadShaderFromFile(vertexFilePath, GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShaderFromFile(fragmentFilePath, GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0) {
        GL_CHECK(glDeleteShader(vertexShader));
        GL_CHECK(glDeleteShader(fragmentShader));
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program == 0) {
        std::cerr << "ERROR::SHADER::PROGRAM::CREATION_FAILED: Could not create shader program." << '\n';
        return 0;
    }
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

