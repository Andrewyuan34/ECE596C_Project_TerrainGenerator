#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath> // Include for sin and cos functions
#include "PerlinNoise.hpp"
#include <glm/glm.hpp>
//#include "camera.hpp" 

#define WIDTH 2048
#define HEIGHT 2048

GLuint getTextureIDForHeight(float height);
GLuint loadTexture(const char *imagepath); 
GLuint textureID;

// 假设纹理ID已经初始化并加载好了
GLuint textureIDs[2]; // 假设有3种不同的纹理

//下一步要做的是，把不同的纹理贴图应用到不同高度的地图上。这样，我们就可以在地图上看到不同的地形，比如山脉、平原、河流等。
int step = 64; // Step size for generating Perlin Noise

PerlinNoise perlinNoise(42, 8); // PerlinNoise object

std::vector<float> heights(WIDTH * HEIGHT / step);

//Camera camera(WIDTH, HEIGHT); // 创建相机对象

// Camera position and angle variables
float lookAtX = 871.724f;
float lookAtY = 2308.83f;
float lookAtZ = 3500.29f;
// Camera position and angle variables
float cameraX = 871.685f;
float cameraY = 2309.52f;
float cameraZ = 3501.01f;

float mouseSpeed = 0.001f;
bool middleButtonPressed = false;
// 定义和初始化鼠标位置变量
float lastX = 400, lastY = 300;
bool firstMouse = true;

// 定义和初始化视角角度变量
float yaw = -90.0f, pitch = 0.0f;

// Movement speed
const float moveSpeed = 0.07f;

int frameCount = 0; // 帧数计数器
int currentTime = 0; // 当前时间
int previousTime = 0; // 上一次计时的时间

void drawString(float x, float y, std::string text);
void generatePerlinNoise();
/*
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 1.0f, 0.1f, 100000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraX, cameraY, cameraZ,   // Eye position (x, y, z)
              lookAtX, lookAtY, lookAtZ,  // Specifies the position of the reference point
              0.0f, 1.0f, 0.0f);           // Up vector (x, y, z)

    // Enable texture mapping
    glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
    
    for (int y = 0; y < HEIGHT - step; y += step) {
        for (int x = 0; x < WIDTH - step; x += step) {
            float tmp_height[4] = {
                heights[y * WIDTH / step + x / step],
                heights[(y + step) * WIDTH / step + x / step],
                heights[(y + step) * WIDTH / step + (x + step) / step],
                heights[y * WIDTH / step + (x + step) / step]
            };

            auto currentTextureID = getTextureIDForHeight(tmp_height[0]);
            glBindTexture(GL_TEXTURE_2D, currentTextureID);
            glBegin(GL_TRIANGLE_FAN);
            glTexCoord2f(static_cast<float>(x) / WIDTH, static_cast<float>(y) / HEIGHT);
            glVertex3f(x, tmp_height[0], y);
            glTexCoord2f(static_cast<float>(x) / WIDTH, static_cast<float>(y + step) / HEIGHT);
            glVertex3f(x, tmp_height[1], y + step);
            glTexCoord2f(static_cast<float>(x + step) / WIDTH, static_cast<float>(y + step) / HEIGHT);
            glVertex3f(x + step, tmp_height[2], y + step);
            glTexCoord2f(static_cast<float>(x + step) / WIDTH, static_cast<float>(y) / HEIGHT);
            glVertex3f(x + step, tmp_height[3], y);
            glEnd();
        }
    }
    

    glDisable(GL_TEXTURE_2D);
    glutSwapBuffers();
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 1.0f, 0.1f, 100000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraX, cameraY, cameraZ,   // Eye position (x, y, z)
              lookAtX, lookAtY, lookAtZ,  // Specifies the position of the reference point
              0.0f, 1.0f, 0.0f);           // Up vector (x, y, z)

    // Enable texture mapping
    glEnable(GL_TEXTURE_2D);

    if (textureID == 0) {
        std::cerr << "Failed to load texture." << std::endl;
        glDisable(GL_TEXTURE_2D); // Disable texture mapping if failed
    } else {
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBegin(GL_QUADS);

    for (int y = 0; y < HEIGHT - step; y += step) { //这里可以查一下有没有什么不同的映射方法，而且似乎得用函数来实现。要不然写在一起太乱了。
        for (int x = 0; x < WIDTH - step; x += step) {
            float texCoordX0 = static_cast<float>(x) / WIDTH;
            float texCoordY0 = static_cast<float>(y) / HEIGHT;
            float texCoordX1 = static_cast<float>(x) / WIDTH;
            float texCoordY1 = static_cast<float>(y + step) / HEIGHT;
            float texCoordX2 = static_cast<float>(x + step) / WIDTH;
            float texCoordY2 = static_cast<float>(y + step) / HEIGHT;
            float texCoordX3 = static_cast<float>(x + step) / WIDTH;
            float texCoordY3 = static_cast<float>(y) / HEIGHT;
    
            glTexCoord2f(texCoordX0, texCoordY0);
            glVertex3f(x, heights[y * WIDTH / step + x / step], y);
            glTexCoord2f(texCoordX1, texCoordY1);
            glVertex3f(x, heights[(y + step) * WIDTH / step + x / step], y + step);
            glTexCoord2f(texCoordX2, texCoordY2);
            glVertex3f(x + step, heights[(y + step) * WIDTH / step + (x + step) / step], y + step);
            glTexCoord2f(texCoordX3, texCoordY3);
            glVertex3f(x + step, heights[y * WIDTH / step + (x + step) / step], y);
        }
    }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
}
*/
void display();
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y);

void render(int value) {
    // 渲染逻辑
    glutPostRedisplay();  // 标记窗口需要重新绘制
    glutTimerFunc(16, render, 0);  // 再次设置定时器
}


void initializeTextures() {
    //textureIDs[0] = loadTexture("Grass1.bmp");
    //textureIDs[1] = loadTexture("Land.bmp");
    //textureIDs[2] = loadTexture("Rock.bmp");
    //if (textureIDs[0] == 0 || textureIDs[1] == 0) {
     //   std::cerr << "Failed to load one or more textures." << std::endl;
    //}
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Perlin Noise Terrain");

    generatePerlinNoise();
    //initializeTextures();

    glEnable(GL_DEPTH_TEST);
    
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutKeyboardFunc(keyboard); // Register keyboard callback function for WASD keys
    glutMouseFunc(mouse); // Register mouse callback function for scroll wheel
    glutMotionFunc(mouseMotion); // Register mouse motion callback function
    glutTimerFunc(16, render, 0);
    glutMainLoop();

    return 0;
}

// Load a BMP image file as a texture
GLuint loadTexture(const char *imagepath) { 
    printf("Reading image %s\n", imagepath);

    // 打开文件
    FILE *file = fopen(imagepath, "rb");
    if (!file) {
        printf("Image could not be opened\n");
        return 0;
    }

    // 读取文件头
    unsigned char header[54];
    if (fread(header, 1, 54, file) != 54) {
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }

    if (header[0] != 'B' || header[1] != 'M') {
        printf("Not a correct BMP file\n");
        fclose(file);
        return 0;
    }

    // 从文件头中获取图像信息
    unsigned int dataPos = *(int*)&(header[0x0A]);
    unsigned int imageSize = *(int*)&(header[0x22]);
    unsigned int width = *(int*)&(header[0x12]);
    unsigned int height = *(int*)&(header[0x16]);

    // 一些BMP文件是没有指定图像大小的，这种情况下，我们可以计算出来
    if (imageSize == 0) imageSize = width * height * 3;
    if (dataPos == 0) dataPos = 54;

    // 创建缓冲区
    unsigned char *data = new unsigned char[imageSize];

    // 读取图像数据到缓冲区中
    fread(data, 1, imageSize, file);

    // 关闭文件
    fclose(file);

    // 创建一个OpenGL纹理
    GLuint textureID;
    glGenTextures(1, &textureID);

    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, textureID);

/*
    // 给纹理设定参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 将图像数据加载到纹理中
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
*/


    // 给纹理设定参数并生成 Mipmap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 设置为使用 Mipmap 的线性过滤器
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 加载图像数据到纹理中，并生成 Mipmap
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_BGR, GL_UNSIGNED_BYTE, data);

    // 释放内存
    delete[] data;

    printf("Successfully loading %s\n", imagepath);

    return textureID;
}

GLuint getTextureIDForHeight(float height) {
    if (height > WIDTH) {
        return textureIDs[0]; // 低高度纹理
    } else if (height > WIDTH * 0.75 && height <= WIDTH) {
        return textureIDs[1]; // 中高度纹理
    } else {
        return textureIDs[2]; // 高高度纹理
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == 3) { // Scroll up
        float directionX = lookAtX - cameraX;
        float directionY = lookAtY - cameraY;
        float directionZ = lookAtZ - cameraZ;
        cameraX += moveSpeed * directionX * 1000;
        cameraY += moveSpeed * directionY * 1000;
        cameraZ += moveSpeed * directionZ * 1000;
        lookAtX += moveSpeed * directionX * 1000;
        lookAtY += moveSpeed * directionY * 1000;
        lookAtZ += moveSpeed * directionZ * 1000;
    } else if (button == 4) { // Scroll down
        float directionX = lookAtX - cameraX;
        float directionY = lookAtY - cameraY;
        float directionZ = lookAtZ - cameraZ;
        cameraX -= moveSpeed * directionX * 1000;
        cameraY -= moveSpeed * directionY * 1000;
        cameraZ -= moveSpeed * directionZ * 1000;
        lookAtX -= moveSpeed * directionX * 1000;
        lookAtY -= moveSpeed * directionY * 1000;
        lookAtZ -= moveSpeed * directionZ * 1000;
    } else if (button == GLUT_MIDDLE_BUTTON) {
        if (state == GLUT_DOWN) {
            middleButtonPressed = true;
            lastX = static_cast<float>(x);
            lastY = static_cast<float>(y);
        } else {
            middleButtonPressed = false;
        }
    }
    glutPostRedisplay();
}

void mouseMotion(int x, int y) {
    if (middleButtonPressed) {
        float sensitivity = 0.1f; // 调整这个值以改变灵敏度

        float xpos = static_cast<float>(x);
        float ypos = static_cast<float>(y);

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // y坐标翻转（从底部到顶部）
        lastX = xpos;
        lastY = ypos;

        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // 限制俯仰角度的范围
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // 更新视角方向向量
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        lookAtX = cameraX + front.x;
        lookAtY = cameraY + front.y;
        lookAtZ = cameraZ + front.z;

        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': // Move camera forward horizontally
            {
                float directionX = lookAtX - cameraX;
                float directionZ = lookAtZ - cameraZ;
                cameraX += moveSpeed * directionX * 1000;
                cameraZ += moveSpeed * directionZ * 1000;
                lookAtX += moveSpeed * directionX * 1000;
                lookAtZ += moveSpeed * directionZ * 1000;
            }
            break;
        case 's': // Move camera backward
            {
                float directionX = lookAtX - cameraX;
                float directionZ = lookAtZ - cameraZ;
                cameraX -= moveSpeed * directionX * 1000;
                cameraZ -= moveSpeed * directionZ * 1000;
                lookAtX -= moveSpeed * directionX * 1000;
                lookAtZ -= moveSpeed * directionZ * 1000;
            }
            break;
        case 'a': // Move camera left
            {
                float directionX = lookAtX - cameraX;
                float directionZ = lookAtZ - cameraZ;
                float perpendicularX = -directionZ;
                float perpendicularZ = directionX;
                cameraX -= moveSpeed * perpendicularX * 1000;
                cameraZ -= moveSpeed * perpendicularZ * 1000;
                lookAtX -= moveSpeed * perpendicularX * 1000;
                lookAtZ -= moveSpeed * perpendicularZ * 1000;
            }
            break;
        case 'd': // Move camera right
            {
                float directionX = lookAtX - cameraX;
                float directionZ = lookAtZ - cameraZ;
                float perpendicularX = -directionZ;
                float perpendicularZ = directionX;
                cameraX += moveSpeed * perpendicularX * 1000;
                cameraZ += moveSpeed * perpendicularZ * 1000;
                lookAtX += moveSpeed * perpendicularX * 1000;
                lookAtZ += moveSpeed * perpendicularZ * 1000;
            }
            break;
        case 'r': // Move camera up
            {
                cameraY += moveSpeed * 1000;
                lookAtY += moveSpeed * 1000;
            }
            break;
        case 'f': // Move camera down
            {
                cameraY -= moveSpeed * 1000;
                lookAtY -= moveSpeed * 1000;
            }
            break;
    }

    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 1.0f, 0.1f, 100000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraX, cameraY, cameraZ,   // Eye position (x, y, z)
              lookAtX, lookAtY, lookAtZ,  // Specifies the position of the reference point
              0.0f, 1.0f, 0.0f);           // Up vector (x, y, z)
    //std:: cout << "cameraX: " << cameraX << " cameraY: " << cameraY << " cameraZ: " << cameraZ << std::endl;
    //std:: cout << "lookAtX: " << lookAtX << " lookAtY: " << lookAtY << " lookAtZ: " << lookAtZ << std::endl;
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);

    for (int z = 0; z < HEIGHT - step; z += step) {
        for (int x = 0; x < WIDTH - step; x += step) {
            // Define vertices of the triangle
            float v1x = x;
            float v1y = heights[z * WIDTH / step + x / step];
            float v1z = z;

            float v2x = x;
            float v2y = heights[(z + step) * WIDTH / step + x / step];
            float v2z = z + step;

            float v3x = x + step;
            float v3y = heights[(z + step) * WIDTH / step + (x + step) / step];
            float v3z = z + step;
            //std::cout << v1y << " " << v2y << " " << v3y << std::endl;

            // Calculate normal vector for lighting (not shown in this simplified example)
            // You should calculate normal vectors for lighting if needed

            // Vertex 1
            glVertex3f(v1x, v1y, v1z);

            // Vertex 2
            glVertex3f(v2x, v2y, v2z);

            // Vertex 3
            glVertex3f(v3x, v3y, v3z);

            // To close the triangle if needed (depends on how you want to draw triangles)
            // glVertex3f(v1x, v1y, v1z);
            // glVertex3f(v3x, v3y, v3z);
            // glVertex3f(v4x, v4y, v4z);
        }
    }

    glEnd();
    glutSwapBuffers();
}


void generatePerlinNoise() {
    for (int z = 0; z < HEIGHT; z += step) {
        for (int x = 0; x < WIDTH; x += step) {
            // Generate Perlin Noise value in range [0, 1]
            double nx = static_cast<double>(x) / WIDTH;
            double nz = static_cast<double>(z) / HEIGHT;
            double ny = 0.0; // Set y-coordinate to 0
            double noiseValue = perlinNoise.generateNoise(nx, ny, nz, 1.0, 1.5, 4.0); //如果想跳着传参数，需要把前面的参数都传一遍，然后再传想要的参数。
            // Store the mapped value in heights array
            heights[(z * WIDTH / step) + x / step] = noiseValue * WIDTH;
        }
    }
}
