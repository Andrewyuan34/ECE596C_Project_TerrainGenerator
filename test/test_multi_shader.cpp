#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>
#include "../shader.hpp"
#include "../PerlinNoise.cpp"
#include "../camera.hpp"

GLuint shaderProgram1;
GLuint VBO, VAO, EBO, texture1, texture2, texture3;
std::vector<GLfloat> vertices;
std::vector<GLuint> indices;
PerlinNoise perlinNoise(42, 4);

float cameraDistance = 40.0f;
float waterLevel = -5.0f; // 设置水平线高度

void init() {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    // 加载并编译着色器
    shaderProgram1 = createShaderProgramFromFile("shader/sand_vertexShader.glsl", "shader/sand_fragmentShader.glsl");

    if (shaderProgram1 == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        return;
    }

    texture1 = loadTexture("Texture/grass.bmp");
    texture2 = loadTexture("Texture/sand.bmp");
    texture3 = loadTexture("Texture/water.bmp");

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

    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW));

    GL_CHECK(glGenBuffers(1, &EBO));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW));

    GLint posAttrib = glGetAttribLocation(shaderProgram1, "aPos");
    GL_CHECK(glEnableVertexAttribArray(posAttrib));
    GL_CHECK(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0));

    GLint texCoordAttrib = glGetAttribLocation(shaderProgram1, "aTexCoord");
    GL_CHECK(glEnableVertexAttribArray(texCoordAttrib));
    GL_CHECK(glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))));

    GL_CHECK(glBindVertexArray(0));
    // 启用混合模式
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void convertMatrix(GLdouble* source, GLfloat* dest) {
    for (int i = 0; i < 16; ++i) {
        dest[i] = static_cast<GLfloat>(source[i]);
    }
}

void display() {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

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

    // 使用着色器程序
    useShaderProgram(shaderProgram1);
    GLint projLoc = glGetUniformLocation(shaderProgram1, "projectionMatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrix);
    GLint viewLoc = glGetUniformLocation(shaderProgram1, "viewMatrix");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);

    // 传递水的高度
    GLint waterLevelLoc = glGetUniformLocation(shaderProgram1, "waterLevel");
    glUniform1f(waterLevelLoc, waterLevel);

    // 绑定纹理和VAO并绘制
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glUniform1i(glGetUniformLocation(shaderProgram1, "texture1"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glUniform1i(glGetUniformLocation(shaderProgram1, "texture2"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture3);
    glUniform1i(glGetUniformLocation(shaderProgram1, "texture3"), 2);

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));

    glutSwapBuffers();
}

void cleanup() {
    deleteShaderProgram(shaderProgram1);
    GL_CHECK(glDeleteVertexArrays(1, &VAO));
    GL_CHECK(glDeleteBuffers(1, &VBO));
    GL_CHECK(glDeleteBuffers(1, &EBO));
    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);
    glDeleteTextures(1, &texture3);
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
    glutCreateWindow("OpenGL Multiple Textures Example");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard); // 注册键盘回调函数
    atexit(cleanup);

    init();
    glutMainLoop();
    return 0;
}
