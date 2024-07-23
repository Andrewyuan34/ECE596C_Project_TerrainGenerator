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
"    gl_Position = vec4(aPos, 1.0);\n"
"}\n";

// 片段着色器源码
const char* fragmentShaderSource = 
"#version 120\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(gl_FragCoord.x / 800.0, gl_FragCoord.y / 600.0, 0.5, 1.0);\n"
"}\n";

GLuint shaderProgram;
GLuint VBO, VAO, EBO;

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
    // 将顶点数据复制到VBO
    // First parameter is the type of the buffer
    // Second parameter is the size of the data in bytes
    // Third parameter is the actual data
    // Fourth parameter is the usage of the data
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    // 生成并绑定EBO
    GL_CHECK(glGenBuffers(1, &EBO));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    // 将索引数据复制到EBO
    // First parameter is the type of the buffer
    // Second parameter is the size of the data in bytes
    // Third parameter is the actual data
    // Fourth parameter is the usage of the data
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    // 配置顶点属性
    GLint posAttrib = glGetAttribLocation(shaderProgram, "aPos"); // 获取顶点属性位置
    GL_CHECK(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0)); // 设置顶点属性指针
    GL_CHECK(glEnableVertexAttribArray(posAttrib)); // 启用顶点属性

    // 解绑VBO和VAO
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0));
    // 注意：EBO在VAO中，解绑VAO时不要解绑EBO

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
    GLint transformLoc = glGetUniformLocation(shaderProgram, "transform"); // 获取uniform变量位置
    GL_CHECK(glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform)); // 设置uniform变量
}

void display() {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    useShaderProgram(shaderProgram);

    // 绑定VAO, EBO在VAO中，不需要再次绑定，但是需要解绑，否则会出错
    // 这里相当于调用VAO里绑定的各种状态
    GL_CHECK(glBindVertexArray(VAO)); 
    // 使用EBO绘制，
    GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));

    glutSwapBuffers();
}

void cleanup() {
    deleteShaderProgram(shaderProgram);
    GL_CHECK(glDeleteVertexArrays(1, &VAO));
    GL_CHECK(glDeleteBuffers(1, &VBO));
    GL_CHECK(glDeleteBuffers(1, &EBO));
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
