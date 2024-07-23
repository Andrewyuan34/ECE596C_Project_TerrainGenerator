#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "../shader.hpp"
#include "../PerlinNoise.cpp"

// 顶点着色器源码
const char* vertexShaderSource = 
"#version 120\n"
"attribute vec3 aPos;\n"
"attribute vec2 aTexCoord;\n"
"varying vec2 vTexCoord;\n"
"varying float vHeight;\n"
"uniform mat4 viewMatrix;\n"
"uniform mat4 projectionMatrix;\n"
"void main()\n"
"{\n"
"    vTexCoord = aTexCoord;\n"
"    vHeight = aPos.y;\n"
"    gl_Position = projectionMatrix * viewMatrix * vec4(aPos, 1.0);\n"
"}\n";

// 片段着色器源码
const char* fragmentShaderSource = 
"#version 120\n"
"varying vec2 vTexCoord;\n"
"varying float vHeight;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"    vec4 texColor = texture2D(texture1, vTexCoord);\n"
"    gl_FragColor = texColor;\n"
"}\n";

GLuint shaderProgram;
GLuint VBO, VAO, EBO, texture;
GLuint texture1;
// 设置顶点数据和缓冲区，并配置顶点属性
std::vector<GLfloat> vertices;
std::vector<GLuint> indices;
PerlinNoise perlinNoise(42, 8); // PerlinNoise object

float cameraDistance = 40.0f; // 初始视角距离

void init() {
    // 初始化GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    // 创建并使用着色器程序
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    useShaderProgram(shaderProgram);

    texture1 = loadTexture("Rock.bmp");
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    const int WIDTH = 2048;
    const int HEIGHT = 2048;
    const int step = 64;

    for (int z = -HEIGHT / 2; z < HEIGHT / 2; z += step) {
        for (int x = -WIDTH / 2; x < WIDTH / 2; x += step) {
            float nx = static_cast<float>(x) / WIDTH;
            float ny = static_cast<float>(z) / HEIGHT;
            float height = perlinNoise.generateNoise(nx, ny, 0.5, 1.0, 1.5, 4.0);
            vertices.push_back(x * 0.1f); // Scale width
            vertices.push_back(height * WIDTH / 30.0f); // Scale height
            vertices.push_back(z * 0.1f); // Scale depth

            // 添加纹理坐标
            vertices.push_back((static_cast<float>(x) + WIDTH / 2) / WIDTH);
            vertices.push_back((static_cast<float>(z) + HEIGHT / 2) / HEIGHT);
        }
    }

    for (int y = 0; y < HEIGHT / step - 1; ++y) {
        for (int x = 0; x < WIDTH / step - 1; ++x) {
            int start = y * (WIDTH / step) + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step);
            indices.push_back(start);
        }
    }

    // 生成并绑定VAO
    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glBindVertexArray(VAO));

    // 生成并绑定VBO
    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW));

    // 生成并绑定EBO
    GL_CHECK(glGenBuffers(1, &EBO));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW));

    // 配置顶点属性
    GLint posAttrib = glGetAttribLocation(shaderProgram, "aPos");
    GL_CHECK(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0));
    GL_CHECK(glEnableVertexAttribArray(posAttrib));

    GLint texCoordAttrib = glGetAttribLocation(shaderProgram, "aTexCoord");
    GL_CHECK(glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))));
    GL_CHECK(glEnableVertexAttribArray(texCoordAttrib));

    // 解绑VAO
    GL_CHECK(glBindVertexArray(0));
}

void convertMatrix(GLdouble* source, GLfloat* dest) {
    for (int i = 0; i < 16; ++i) {
        dest[i] = static_cast<GLfloat>(source[i]);
    }
}

void display() {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    useShaderProgram(shaderProgram);

    // 设置投影矩阵
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 5.0f, 1000.0f);
    GLdouble projectionMatrixD[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrixD);
    GLfloat projectionMatrix[16];
    convertMatrix(projectionMatrixD, projectionMatrix);

    // 设置视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraDistance, cameraDistance, cameraDistance,  // 摄像机位置 (x, y, z)
              0.0f, 0.0f, 0.0f,  // 目标位置 (x, y, z)
              0.0f, 1.0f, 0.0f); // 上向量 (x, y, z)
    GLdouble viewMatrixD[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, viewMatrixD);
    GLfloat viewMatrix[16];
    convertMatrix(viewMatrixD, viewMatrix);

    // 传递投影矩阵和视图矩阵给着色器
    GLint projLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrix);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "viewMatrix");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    // 绑定VAO
    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));

    glutSwapBuffers();
}

void cleanup() {
    deleteShaderProgram(shaderProgram);
    GL_CHECK(glDeleteVertexArrays(1, &VAO));
    GL_CHECK(glDeleteBuffers(1, &VBO));
    GL_CHECK(glDeleteBuffers(1, &EBO));
    glDeleteTextures(1, &texture1);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': // 放大视角
            cameraDistance -= 1.0f;
            glutPostRedisplay();
            break;
        case 's': // 缩小视角
            cameraDistance += 1.0f;
            glutPostRedisplay();
            break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Perlin Noise Test");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard); // 注册键盘回调函数
    atexit(cleanup);

    init();
    glutMainLoop();

    return 0;
}
