#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>
#include "../shader.hpp"
#include "../PerlinNoise.cpp"
#include "../camera.hpp"
#include "../command_line_parser.hpp"

//找到高度的最大值和最小值
//然后筛选出边界的顶点
//但问题在于EBO不一样了，这个怎么解决？


GLuint shaderProgram1;
GLuint cubeShaderProgram;
GLuint VBO, VAO, EBO, texture1, texture2, texture3;
std::vector<GLfloat> vertices;
std::vector<GLuint> indices;
PerlinNoise perlinNoise(17, 4); //asdddddddddddddddd

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

GLfloat lightPosY = 100.0f; // 初始化光源Y坐标

std::vector<GLfloat> cubeVertices = {
    // Positions          
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f
};

std::vector<GLuint> cubeIndices = {
    // Front face
    0, 1, 2,
    2, 3, 0,
    // Back face
    4, 5, 6,
    6, 7, 4,
    // Left face
    0, 4, 7,
    7, 3, 0,
    // Right face
    1, 5, 6,
    6, 2, 1,
    // Top face
    3, 2, 6,
    6, 7, 3,
    // Bottom face
    0, 1, 5,
    5, 4, 0
};

GLuint cubeVAO, cubeVBO, cubeEBO;


void initCube() {
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(GLfloat), &cubeVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(GLuint), &cubeIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}




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

void init(double frequency, int octave, double amplitude, double persistence, double lacunarity) {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    // 加载并编译着色器
    shaderProgram1 = createShaderProgramFromFile("shader/sand_vertexShader.glsl", "shader/sand_fragmentShader.glsl");
    cubeShaderProgram = createShaderProgramFromFile("shader/cube_vertex_shader.glsl", "shader/cube_fragment_shader.glsl");

    if (shaderProgram1 == 0 || cubeShaderProgram == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        return;
    }

    texture1 = loadTexture("Texture/grass.bmp");
    texture2 = loadTexture("Texture/sand.bmp");
    texture3 = loadTexture("Texture/water.bmp");

    // 设置环境光颜色和强度
    glUseProgram(shaderProgram1);
    GLint ambientLightLoc = glGetUniformLocation(shaderProgram1, "ambientLight");
    glUniform3f(ambientLightLoc, 0.3f, 0.3f, 0.3f); // 这里设置环境光为灰色，强度为 0.3

    std::vector<float> height_map(WIDTH * HEIGHT);
    int i = 0;
    int j = 0;
    float minHeight = std::numeric_limits<float>::max();
    float maxHeight = std::numeric_limits<float>::min();
    std::vector<float> height_array;
    height_array.reserve(WIDTH * HEIGHT);
    // 生成地形顶点数据和索引
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
    std::vector<GLfloat> vertices;
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
    // 生成水面平面的顶点数据和索引
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

    std::vector<GLuint> indices;
    // 生成地形索引数据
    for (int y = 0; y < HEIGHT / step - 1; ++y) {
        for (int x = 0; x < WIDTH / step - 1; ++x) {
            int start = y * (WIDTH / step) + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step);
            indices.push_back(start);
            //std::cout << start << " " << start + 1 << " " << start + WIDTH / step + 1 << " " << start + WIDTH / step << std::endl;
        }
    }

    // 生成水面索引数据
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
            //std::cout << start << " " << start + 1 << " " << start + WIDTH / step + 1 << " " << start + WIDTH / step << std::endl;
        }
    }

    // 计算顶点法线
    std::vector<GLfloat> normals;
    computeVertexNormals(vertices, indices, normals);

    // 将法线添加到顶点数据中
    std::vector<GLfloat> verticesWithNormals;
    for (size_t i = 0; i < vertices.size() / 6; ++i) {
        verticesWithNormals.push_back(vertices[i * 6]);     // x
        verticesWithNormals.push_back(vertices[i * 6 + 1]); // y
        verticesWithNormals.push_back(vertices[i * 6 + 2]); // z
        verticesWithNormals.push_back(normals[i * 6]);      // nx
        verticesWithNormals.push_back(normals[i * 6 + 1]);  // ny
        verticesWithNormals.push_back(normals[i * 6 + 2]);  // nz
        verticesWithNormals.push_back(vertices[i * 6 + 3]); // u
        verticesWithNormals.push_back(vertices[i * 6 + 4]); // v
        verticesWithNormals.push_back(vertices[i * 6 + 5]); // height
        std::cout << normals[i * 6] << " " << normals[i * 6 + 1] << " " << normals[i * 6 + 2] << std::endl;
    }

    // 生成并绑定VAO
    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glBindVertexArray(VAO));

    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, verticesWithNormals.size() * sizeof(GLfloat), verticesWithNormals.data(), GL_STATIC_DRAW));

    GL_CHECK(glGenBuffers(1, &EBO));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW));

    GLint posAttrib = glGetAttribLocation(shaderProgram1, "aPos");
    GL_CHECK(glEnableVertexAttribArray(posAttrib));
    GL_CHECK(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0));

    GLint normalAttrib = glGetAttribLocation(shaderProgram1, "aNormal");
    GL_CHECK(glEnableVertexAttribArray(normalAttrib));
    GL_CHECK(glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))));

    GLint texCoordAttrib = glGetAttribLocation(shaderProgram1, "aTexCoord");
    GL_CHECK(glEnableVertexAttribArray(texCoordAttrib));
    GL_CHECK(glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))));

    GLint heightAttrib = glGetAttribLocation(shaderProgram1, "aHeight");
    GL_CHECK(glEnableVertexAttribArray(heightAttrib));
    GL_CHECK(glVertexAttribPointer(heightAttrib, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))));

    GL_CHECK(glBindVertexArray(0));

    initCube();

    // 设置面剔除和深度测试
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 启用混合模式
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 初始化 FPS 计时器
    lastTime = std::chrono::high_resolution_clock::now();
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
    gluPerspective(45.0f, 800.0f / 600.0f, 10.0f, 1000.0f);
    GLdouble projectionMatrixD[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrixD);
    GLfloat projectionMatrix[16];
    convertMatrix(projectionMatrixD, projectionMatrix);

    // 设置视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera.getCameraPos().x, camera.getCameraPos().y, camera.getCameraPos().z,  // 摄像机位置 (x, y, z)
                camera.getCameraPos().x + camera.getCameraFront().x, camera.getCameraPos().y + camera.getCameraFront().y, camera.getCameraPos().z + camera.getCameraFront().z,  // 目标位置 (x, y, z)
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

    // 设置光源和摄像机位置
    GLint lightPosLoc = glGetUniformLocation(shaderProgram1, "lightPos");
    glUniform3f(lightPosLoc, 100.0f, lightPosY, 100.0f);
    std::cout << "lightPosY: " << lightPosY << std::endl;
    GLint viewPosLoc = glGetUniformLocation(shaderProgram1, "viewPos");
    glUniform3f(viewPosLoc, camera.getCameraPos().x, camera.getCameraPos().y, camera.getCameraPos().z);

    // 绘制地形
    GLint useWaterTextureLoc = glGetUniformLocation(shaderProgram1, "useWaterTexture");
    glUniform1i(useWaterTextureLoc, GL_FALSE); // 禁用水纹理

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glUniform1i(glGetUniformLocation(shaderProgram1, "texture1"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glUniform1i(glGetUniformLocation(shaderProgram1, "texture2"), 1);

    // 根据 showWireframe 变量设置绘制模式
    if (showWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, (HEIGHT / step - 1) * (WIDTH / step - 1) * 6, GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));

    // 绘制水面
    glUniform1i(useWaterTextureLoc, GL_TRUE); // 启用水纹理（尽管不使用纹理，我们仍然使用这个标志来控制渲染）

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, (HEIGHT / step - 1) * (WIDTH / step - 1) * 6, GL_UNSIGNED_INT, (GLvoid*)((HEIGHT / step - 1) * (WIDTH / step - 1) * 6 * sizeof(GLuint))));
    GL_CHECK(glBindVertexArray(0));

    // 使用正方体着色器程序,这个部分可以先让gpt写一个最简单的立方体光源程序，然后把这两个程序结合到一块就行了。
    glUseProgram(cubeShaderProgram);

    GLint modelLoc = glGetUniformLocation(cubeShaderProgram, "model");
    GLint viewLocCube = glGetUniformLocation(cubeShaderProgram, "view");
    GLint projLocCube = glGetUniformLocation(cubeShaderProgram, "projection");
    GLfloat modelMatrix[16] = {
        5.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 5.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 5.0f, 0.0f,
        100.0f, lightPosY, 100.0f, 1.0f
    };

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(viewLocCube, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(projLocCube, 1, GL_FALSE, projectionMatrix);

    GLint lightColorLoc = glGetUniformLocation(cubeShaderProgram, "lightColor");
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // 发光正方体的颜色

    GL_CHECK(glBindVertexArray(cubeVAO));
    GL_CHECK(glDrawElements(GL_TRIANGLES, cubeIndices.size(), GL_UNSIGNED_INT, 0));
    GL_CHECK(glBindVertexArray(0));

    // 更新 FPS
    updateFPS();
    glutSwapBuffers();
    
}





void cleanup() {
    deleteShaderProgram(shaderProgram1);
    deleteShaderProgram(cubeShaderProgram);

    // 删除地形的 VAO、VBO 和 EBO
    GL_CHECK(glDeleteVertexArrays(1, &VAO));
    GL_CHECK(glDeleteBuffers(1, &VBO));
    GL_CHECK(glDeleteBuffers(1, &EBO));

    // 删除立方体的 VAO、VBO 和 EBO
    GL_CHECK(glDeleteVertexArrays(1, &cubeVAO));
    GL_CHECK(glDeleteBuffers(1, &cubeVBO));
    GL_CHECK(glDeleteBuffers(1, &cubeEBO));

    // 删除纹理
    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);
    glDeleteTextures(1, &texture3);
}


/*void keyboard(unsigned char key, int x, int y) {
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
}*/

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '1':
            showWireframe = !showWireframe;
            break;
        case '2': // 光源上升
            lightPosY += 10.0f;
            break;
        case '3': // 光源下降
            lightPosY -= 10.0f;
            break;
        default:
            camera.keyboard(key, x, y);
            break;
    }
    glutPostRedisplay(); // 刷新显示
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
    //waterLevel = amplitude; // 假设水面高度用的是amplitude参数
    std::cout << "frequency: " << frequency << " octave: " << octave << " amplitude: " << amplitude << " persistence: " << persistence << " lacunarity: " << lacunarity << std::endl;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Multiple Textures Example");

    glutDisplayFunc(display);
    //glutIdleFunc(glutPostRedisplay);
    glutKeyboardFunc(keyboard); // 注册键盘回调函数
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    atexit(cleanup);

    init(frequency, octave, amplitude, persistence, lacunarity);
    glutMainLoop();
    return 0;
}
