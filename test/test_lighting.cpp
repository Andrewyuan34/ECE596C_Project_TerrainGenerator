#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring> // for memcpy
#include <chrono>
#include "../shader.hpp"
#include "../PerlinNoise.hpp"
#include "../camera.hpp"
#include "../command_line_parser.hpp"

GLuint shaderProgram1;
GLuint VBO, VAO, EBO, texture1, texture2, texture3, lightVAO, lightVBO;
std::vector<GLfloat> vertices;
std::vector<GLuint> indices;
PerlinNoise perlinNoise(14, 4);

float cameraDistance = 100.0f;
float waterLevel = -5.0f; // 设置水平线高度

const int WIDTH = 4096;
const int HEIGHT = WIDTH;
const int step = WIDTH / 32;

Camera camera({0, 0, 100});

// FPS 相关变量
int frameCount = 0;
double fps = 0.0;
std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;

// 网格显示控制变量
bool showWireframe = false;

void updateFPS() {
    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = currentTime - lastTime;

    if (elapsedTime.count() >= 1.0) {
        fps = frameCount / elapsedTime.count();
        frameCount = 0;
        lastTime = currentTime;

        std::string title = "OpenGL Multiple Textures Example - FPS: " + std::to_string(fps);
        glutSetWindowTitle(title.c_str());
    }
}

void createPerspectiveMatrix(float* matrix, float fov, float aspect, float near, float far) {
    float tanHalfFov = tan(fov / 2.0f);
    float range = near - far;

    matrix[0] = 1.0f / (aspect * tanHalfFov);
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 1.0f / tanHalfFov;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = (near + far) / range;
    matrix[11] = -1.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = (2.0f * near * far) / range;
    matrix[15] = 0.0f;
}

void createLookAtMatrix(float* matrix, Vec eye, Vec center, Vec up) {
    Vec f = { center.x - eye.x, center.y - eye.y, center.z - eye.z };
    f = camera.normalize(f);

    Vec u = camera.normalize(up);
    Vec s = camera.crossProduct(f, u);
    u = camera.crossProduct(s, f);

    matrix[0] = s.x;
    matrix[1] = u.x;
    matrix[2] = -f.x;
    matrix[3] = 0.0f;

    matrix[4] = s.y;
    matrix[5] = u.y;
    matrix[6] = -f.y;
    matrix[7] = 0.0f;

    matrix[8] = s.z;
    matrix[9] = u.z;
    matrix[10] = -f.z;
    matrix[11] = 0.0f;

    matrix[12] = -camera.dot(s, eye);
    matrix[13] = -camera.dot(u, eye);
    matrix[14] = camera.dot(f, eye);
    matrix[15] = 1.0f;
}

void init(double frequency, int octave, double amplitude, double persistence, double lacunarity) {
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

    // 设置环境光颜色和强度
    glUseProgram(shaderProgram1);
    GLint ambientLightLoc = glGetUniformLocation(shaderProgram1, "ambientLight");
    glUniform3f(ambientLightLoc, 1.0f, 1.0f, 1.0f); // 这里设置环境光为灰色，强度为 0.3

    std::vector<float> height_map(WIDTH * HEIGHT);
    int i = 0;
    int j = 0;
    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = std::numeric_limits<float>::min();
    std::vector<float> height_array;
    height_array.reserve(WIDTH * HEIGHT);
    for (int z = -HEIGHT / 2; z < HEIGHT / 2; z += step) {
        for (int x = -WIDTH / 2; x < WIDTH / 2; x += step) {
            float nx = static_cast<float>(x) / WIDTH;
            float ny = static_cast<float>(z) / HEIGHT;
            float height = perlinNoise.generateNoise(nx, ny, 0.5, frequency, amplitude, octave, persistence, lacunarity) + 1.5;
            height_array.push_back(height);
            if (height < minHeight) minHeight = height;
            if (height > maxHeight) maxHeight = height;
        }
    }
    waterLevel = ((maxHeight - minHeight) * 0.3f + minHeight); // 设置水面高度为地形高度的 30%
    i = 0;
    for (int z = -HEIGHT / 2; z < HEIGHT / 2; z += step) {
        for (int x = -WIDTH / 2; x < WIDTH / 2; x += step) {
            height_array[i] = perlinNoise.adjustNoiseForTerrainShape(height_array[i], x, z, WIDTH, HEIGHT, step, waterLevel);
            float scaledHeight = height_array[i] * WIDTH / 60.0f;
            vertices.push_back(x * 0.1f); // 宽度缩放
            vertices.push_back(scaledHeight); // 高度缩放
            vertices.push_back(z * 0.1f); // 深度缩放
            i++;
            height_map[j++] = scaledHeight;
            // 添加纹理坐标
            vertices.push_back((static_cast<float>(x) + WIDTH / 2) / WIDTH);
            vertices.push_back((static_cast<float>(z) + HEIGHT / 2) / HEIGHT);

            // 添加地形高度（此时设置为0, 水面部分会在之后更新）
            vertices.push_back(0.0f);
        }
    }

    j = 0;
    waterLevel = waterLevel * WIDTH / 60.0f;
    for (int z = -HEIGHT / 2; z < HEIGHT / 2; z += step) {
        for (int x = -WIDTH / 2; x < WIDTH / 2; x += step) {
            vertices.push_back(x * 0.1f); // 宽度缩放
            vertices.push_back(waterLevel); // 固定高度为水平面
            vertices.push_back(z * 0.1f); // 深度缩放

            // 添加纹理坐标
            vertices.push_back((static_cast<float>(x) + WIDTH / 2) / WIDTH);
            vertices.push_back((static_cast<float>(z) + HEIGHT / 2) / HEIGHT);

            // 添加地形高度，用于判断是否显示水面
            vertices.push_back(height_map[j++]);
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

    int waterOffset = (HEIGHT / step) * (WIDTH / step);
    for (int y = 0; y < HEIGHT / step - 1; ++y) {
        for (int x = 0; x < WIDTH / step - 1; ++x) {
            int start = waterOffset + y * (WIDTH / step) + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step);
            indices.push_back(start);
        }
    }

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    GLint posAttrib = glGetAttribLocation(shaderProgram1, "aPos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);

    GLint texCoordAttrib = glGetAttribLocation(shaderProgram1, "aTexCoord");
    glEnableVertexAttribArray(texCoordAttrib);
    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    GLint heightAttrib = glGetAttribLocation(shaderProgram1, "aHeight");
    glEnableVertexAttribArray(heightAttrib);
    glVertexAttribPointer(heightAttrib, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));

    glBindVertexArray(0);

    GLfloat lightVertices[] = {
        -0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
        -0.1f,  0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f,

        -0.1f, -0.1f,  0.1f,
         0.1f, -0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f, -0.1f,  0.1f,

        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f,
        -0.1f, -0.1f, -0.1f,
        -0.1f, -0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,

         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,

        -0.1f, -0.1f, -0.1f,
         0.1f, -0.1f, -0.1f,
         0.1f, -0.1f,  0.1f,
         0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f, -0.1f,

        -0.1f,  0.1f, -0.1f,
         0.1f,  0.1f, -0.1f,
         0.1f,  0.1f,  0.1f,
         0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f
    };

    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize FPS timer
    lastTime = std::chrono::high_resolution_clock::now();
}

void display() {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    // Create and set the projection matrix
    float projectionMatrix[16];
    createPerspectiveMatrix(projectionMatrix, 45.0f * M_PI / 180.0f, 800.0f / 600.0f, 10.0f, 1000.0f);

    // Create and set the view matrix
    Vec cameraPos = camera.getCameraPos();
    Vec cameraFront = camera.getCameraFront();
    Vec up = {0.0f, 1.0f, 0.0f};
    float viewMatrix[16];
    createLookAtMatrix(viewMatrix, cameraPos, {cameraPos.x + cameraFront.x, cameraPos.y + cameraFront.y, cameraPos.z + cameraFront.z}, up);

    // Set model matrix to identity
    float modelMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Use shader program
    useShaderProgram(shaderProgram1);
    GLint projLoc = glGetUniformLocation(shaderProgram1, "projectionMatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectionMatrix);
    GLint viewLoc = glGetUniformLocation(shaderProgram1, "viewMatrix");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);
    GLint modelLoc = glGetUniformLocation(shaderProgram1, "modelMatrix");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix);

    // Set light position and color
    float lightPos[] = {0.0f, 10.0f, 0.0f};
    float lightColor[] = {1.0f, 1.0f, 1.0f};
    GLint lightPosLoc = glGetUniformLocation(shaderProgram1, "lightPos");
    glUniform3fv(lightPosLoc, 1, lightPos);
    GLint lightColorLoc = glGetUniformLocation(shaderProgram1, "lightColor");
    glUniform3fv(lightColorLoc, 1, lightColor);

    // Pass camera position
    float viewPos[] = {cameraPos.x, cameraPos.y, cameraPos.z};
    GLint viewPosLoc = glGetUniformLocation(shaderProgram1, "viewPos");
    glUniform3fv(viewPosLoc, 1, viewPos);

    // Pass water level
    GLint waterLevelLoc = glGetUniformLocation(shaderProgram1, "waterLevel");
    glUniform1f(waterLevelLoc, waterLevel);

    // Draw terrain
    GLint useWaterTextureLoc = glGetUniformLocation(shaderProgram1, "useWaterTexture");
    glUniform1i(useWaterTextureLoc, GL_FALSE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glUniform1i(glGetUniformLocation(shaderProgram1, "texture1"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glUniform1i(glGetUniformLocation(shaderProgram1, "texture2"), 1);

    if (showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, (HEIGHT / step - 1) * (WIDTH / step - 1) * 6, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));

    // Draw water surface
    glUniform1i(useWaterTextureLoc, GL_TRUE);

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, (HEIGHT / step - 1) * (WIDTH / step - 1) * 6, GL_UNSIGNED_INT, (GLvoid*)((HEIGHT / step - 1) * (WIDTH / step - 1) * 6 * sizeof(GLuint))));
    GL_CHECK(glBindVertexArray(0));

    // Draw light object
    useShaderProgram(shaderProgram1);
    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Update FPS
    updateFPS();
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
    GL_CHECK(glDeleteVertexArrays(1, &lightVAO));
    GL_CHECK(glDeleteBuffers(1, &lightVBO));
}

void keyboard(unsigned char key, int x, int y) {
    if (key == '1') {
        showWireframe = !showWireframe;
    }
    camera.keyboard(key, x, y);
}

void mouse(int button, int state, int x, int y) {
    camera.mouse(button, state, x, y);
}

void mouseMotion(int x, int y) {
    camera.mouseMotion(x, y);
}

int main(int argc, char** argv) {
    // 使用CommandLineParser解析命令行参数
    CommandLineParser parser;
    try {
        parser.parse(argc, argv);
    } catch (const std::runtime_error& e) {
        return 1;
    }

    // 设置解析的参数
    double frequency = parser.getFrequency();
    int octave = parser.getOctave();
    double amplitude = parser.getAmplitude();
    double persistence = parser.getPersistence();
    double lacunarity = parser.getLacunarity();
    std::cout << "frequency: " << frequency << " octave: " << octave << " amplitude: " << amplitude << " persistence: " << persistence << " lacunarity: " << lacunarity << std::endl;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Multiple Textures Example");

    glutDisplayFunc(display);
    glutIdleFunc(glutPostRedisplay);
    glutKeyboardFunc(keyboard); // 注册键盘回调函数
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    atexit(cleanup);

    init(frequency, octave, amplitude, persistence, lacunarity);
    glutMainLoop();
    return 0;
}
