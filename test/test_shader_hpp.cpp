#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include "../shader.hpp"

// 顶点着色器源码
const char* vertexShaderSource = 
"#version 120\n"
"attribute vec3 aPos;\n"
"uniform mat4 transform;\n"
"void main()\n"
"{\n"
"    gl_Position = transform * vec4(aPos, 1.0);\n"
"}\n";

// 片段着色器源码
const char* fragmentShaderSource = 
"#version 120\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(gl_FragCoord.x / 800.0, gl_FragCoord.y / 600.0, 0.5, 1.0);\n"
"}\n";

GLuint shaderProgram;
GLuint VBO, VAO;

void init() {
    // 初始化GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    // 创建并使用着色器程序，测试 createShaderProgram 和 useShaderProgram 函数
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    useShaderProgram(shaderProgram);

    // 设置顶点数据和缓冲区，并配置顶点属性
    GLfloat vertices[] = {
        0.5f,  0.5f, 0.0f,  // Top Right
        0.5f, -0.5f, 0.0f,  // Bottom Right
       -0.5f, -0.5f, 0.0f,  // Bottom Left
       -0.5f,  0.5f, 0.0f   // Top Left 
    };

    GLuint indices[] = {  // 注意我们从0开始
        0, 1, 3,  // 第一个三角形
        1, 2, 3   // 第二个三角形
    };

    // 生成并绑定VAO
    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glBindVertexArray(VAO));

    // 生成并绑定VBO
    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // 配置顶点属性
    GLint posAttrib = glGetAttribLocation(shaderProgram, "aPos");
    GL_CHECK(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0));
    GL_CHECK(glEnableVertexAttribArray(posAttrib));

    // 解绑VBO和VAO
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));

    // 设置变换矩阵（旋转）
    float angle = 45.0f * M_PI / 180.0f; // 45度转为弧度
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    GLfloat transform[] = {
        cosAngle, -sinAngle, 0.0f, 0.0f,
        sinAngle,  cosAngle, 0.0f, 0.0f,
        0.0f,      0.0f,     1.0f, 0.0f,
        0.0f,      0.0f,     0.0f, 1.0f
    };
    GLint transformLoc = glGetUniformLocation(shaderProgram, "transform");
    GL_CHECK(glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform));

    // 测试错误处理函数，尝试获取不存在的uniform变量
    GLint testUniformLoc = glGetUniformLocation(shaderProgram, "nonexistent");
    if (testUniformLoc == -1) {
        std::cerr << "Uniform 'nonexistent' not found (as expected)." << std::endl;
    }

    // 测试从文件加载着色器
    std::string shaderCode = readShaderSource("nonexistent_shader.glsl");
    if (shaderCode.empty()) {
        std::cerr << "Failed to read nonexistent shader file (as expected)." << std::endl;
    }
}

void display() {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    useShaderProgram(shaderProgram);

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
    GL_CHECK(glBindVertexArray(0));

    glutSwapBuffers();
}

void cleanup() {
    deleteShaderProgram(shaderProgram);
    GL_CHECK(glDeleteVertexArrays(1, &VAO));
    GL_CHECK(glDeleteBuffers(1, &VBO));
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Shader Test");

    glutDisplayFunc(display);
    atexit(cleanup);

    init();
    glutMainLoop();

    return 0;
}
