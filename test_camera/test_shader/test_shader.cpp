#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// 窗口大小
const int WIDTH = 800;
const int HEIGHT = 600;

// 顶点数据
GLfloat vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

GLuint shaderProgram;
GLuint VAO, VBO;

// 检查着色器编译错误
void checkShaderCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "| ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "| ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

// 从文件读取着色器源码
std::string readShaderSource(const std::string& filePath) {
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

// 编译着色器
GLuint loadShader(const GLchar* shaderSource, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    checkShaderCompileErrors(shader, shaderType == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    return shader;
}

// 创建和链接着色器程序
GLuint createShaderProgram(const std::string& vertexFilePath, const std::string& fragmentFilePath) {
    std::string vertexCode = readShaderSource(vertexFilePath);
    std::string fragmentCode = readShaderSource(fragmentFilePath);

    if (vertexCode.empty() || fragmentCode.empty()) {
        return 0;
    }

    GLuint vertexShader = loadShader(vertexCode.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader(fragmentCode.c_str(), GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    checkShaderCompileErrors(program, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 绑定VAO
    glBindVertexArray(VAO);

    // 绘制三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 解绑VAO
    glBindVertexArray(0);

    glutSwapBuffers();
}

void initGL() {
    // 初始化GLEW
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(glewStatus) << std::endl;
        exit(1);
    }

    // 检查OpenGL版本
    if (!GLEW_VERSION_2_1) {
        std::cerr << "OpenGL 2.1 not supported" << std::endl;
        exit(1);
    }

    // 设置视口
    glViewport(0, 0, WIDTH, HEIGHT);
}

void initShaders() {
    // 从文件加载并编译着色器
    shaderProgram = createShaderProgram("vertex_shader.glsl", "fragment_shader.glsl");
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        exit(1);
    }
}

void initBuffers() {
    // 生成并绑定VAO和VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

int main(int argc, char** argv) {
    // 初始化GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("OpenGL GLUT Test");

    // 初始化OpenGL
    initGL();

    // 初始化着色器
    initShaders();

    // 初始化缓冲区
    initBuffers();

    // 注册显示回调函数
    glutDisplayFunc(display);

    // 注册清理回调函数
    atexit(cleanup);

    // 进入GLUT主循环
    glutMainLoop();

    return 0;
}
