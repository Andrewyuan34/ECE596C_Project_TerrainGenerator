#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <filesystem>

// 着色器源码
const char* toonVertexShaderSource = R"(
#version 120
uniform vec3 lightDir;
varying float intensity;

void main()
{
    intensity = dot(lightDir, gl_Normal);
    gl_Position = ftransform();
}
)";

const char* toonFragmentShaderSource = R"(
#version 120
varying float intensity;

void main()
{
    vec4 color;
    if (intensity > 0.95)
        color = vec4(1.0, 0.5, 0.5, 1.0); // 最亮的颜色
    else if (intensity > 0.5)
        color = vec4(0.6, 0.3, 0.3, 1.0); // 中等亮度
    else if (intensity > 0.25)
        color = vec4(0.4, 0.2, 0.2, 1.0); // 较暗
    else
        color = vec4(0.2, 0.1, 0.1, 1.0); // 最暗的颜色
    
    gl_FragColor = color;
}

)";

GLuint shaderProgram;
float cameraDistance = 3.0f;

struct Vec {
    float x, y, z;
};

Vec lightPos = {1.0f, 1.0f, 1.0f};
Vec lightColor = {1.0f, 1.0f, 1.0f};
Vec objectColor = {0.5f, 0.5f, 1.0f};

// 读取编译着色器
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

// 创建着色器程序
GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, toonVertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, toonFragmentShaderSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void init() {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return;
    }

    shaderProgram = createShaderProgram();
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        return;
    }
}

void setUniformMat4(GLint location, const GLfloat* matrix) {
    glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
}

void setUniformVec3(GLint location, const Vec& vec) {
    glUniform3f(location, vec.x, vec.y, vec.z);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    // 设置投影矩阵
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    GLfloat projectionMatrix[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);

    // 设置视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraDistance, cameraDistance, cameraDistance, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    GLfloat viewMatrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);

    // 设置模型矩阵
    GLfloat modelMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    setUniformMat4(projLoc, projectionMatrix);
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    setUniformMat4(viewLoc, viewMatrix);
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    setUniformMat4(modelLoc, modelMatrix);
    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    setUniformVec3(lightPosLoc, lightPos);
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    setUniformVec3(viewPosLoc, {cameraDistance, cameraDistance, cameraDistance});
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    setUniformVec3(lightColorLoc, lightColor);
    GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    setUniformVec3(objectColorLoc, objectColor);

    glutSolidTeapot(1.0);
    glutSwapBuffers();
}

void cleanup() {
    glDeleteProgram(shaderProgram);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': // 放大视角
            cameraDistance -= 1.0f;
            if (cameraDistance < 1.0f) cameraDistance = 1.0f;
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
    glutCreateWindow("OpenGL Toon Shading Example");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard); // 注册键盘回调函数
    atexit(cleanup);

    init();
    glutMainLoop();
    return 0;
}

