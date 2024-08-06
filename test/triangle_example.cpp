#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>

// 顶点着色器源码
const char* vertexShaderSource = 
    "#version 120\n"
    "attribute vec3 position;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(position, 1.0);\n"
    "}\n";

// 片段着色器源码 - 绘制红色三角形
const char* fragmentShaderSource = 
    "#version 120\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n" // 红色
    "}\n";

// 片段着色器源码 - 边缘检测
const char* edgeDetectionFragmentShaderSource = 
    "#version 120\n"
    "uniform sampler2D tex;\n"
    "void main()\n"
    "{\n"
    "    vec2 uv = gl_FragCoord.xy / vec2(400.0, 400.0); // 400.0 是窗口大小\n"
    "    vec4 center = texture2D(tex, uv);\n"
    "    vec4 left = texture2D(tex, uv + vec2(-1.0, 0.0) / vec2(400.0, 400.0));\n"
    "    vec4 right = texture2D(tex, uv + vec2(1.0, 0.0) / vec2(400.0, 400.0));\n"
    "    vec4 bottom = texture2D(tex, uv + vec2(0.0, -1.0) / vec2(400.0, 400.0));\n"
    "    vec4 top = texture2D(tex, uv + vec2(0.0, 1.0) / vec2(400.0, 400.0));\n"
    "    vec4 edge = abs(center - left) + abs(center - right) + abs(center - bottom) + abs(center - top);\n"
    "    gl_FragColor = vec4(edge.rgb, 1.0);\n" // 边缘强度为灰度值
    "}\n";

GLuint triangleProgram;
GLuint edgeDetectionProgram;
GLint triangleAttributePosition;
GLint edgeDetectionAttributePosition;
GLuint framebuffer;
GLuint renderedTexture;

void initGL()
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Error: GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
        exit(1);
    }

    std::cout << "Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
}

void initShader()
{
    // 创建顶点着色器对象
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // 创建片段着色器对象 - 绘制红色三角形
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 创建着色器程序对象 - 绘制红色三角形
    triangleProgram = glCreateProgram();
    glAttachShader(triangleProgram, vertexShader);
    glAttachShader(triangleProgram, fragmentShader);
    glLinkProgram(triangleProgram);

    // 使用着色器程序 - 绘制红色三角形
    glUseProgram(triangleProgram);

    // 获取顶点位置属性位置 - 绘制红色三角形
    triangleAttributePosition = glGetAttribLocation(triangleProgram, "position");

    // 创建片段着色器对象 - 边缘检测
    GLuint edgeDetectionFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(edgeDetectionFragmentShader, 1, &edgeDetectionFragmentShaderSource, NULL);
    glCompileShader(edgeDetectionFragmentShader);

    // 创建着色器程序对象 - 边缘检测
    edgeDetectionProgram = glCreateProgram();
    glAttachShader(edgeDetectionProgram, vertexShader);
    glAttachShader(edgeDetectionProgram, edgeDetectionFragmentShader);
    glLinkProgram(edgeDetectionProgram);

    // 使用着色器程序 - 边缘检测
    glUseProgram(edgeDetectionProgram);

    // 获取顶点位置属性位置 - 边缘检测
    edgeDetectionAttributePosition = glGetAttribLocation(edgeDetectionProgram, "position");
}

void createFramebuffer(int width, int height)
{
    // 创建帧缓冲对象
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // 创建纹理附件
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

    // 检查帧缓冲完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;

    // 解绑帧缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void display()
{
    // 绘制到帧缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, 400, 400); // 设置视口大小为窗口大小

    glClear(GL_COLOR_BUFFER_BIT);

    // 使用绘制红色三角形的着色器程序
    glUseProgram(triangleProgram);

    // 定义三角形顶点数据
    GLfloat vertices[] = {
        0.0f,  0.5f, 0.0f,
       -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    // 启用顶点属性数组
    glEnableVertexAttribArray(triangleAttributePosition);
    glVertexAttribPointer(triangleAttributePosition, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    // 绘制三角形
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 禁用顶点属性数组
    glDisableVertexAttribArray(triangleAttributePosition);

    // 解绑帧缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 绘制到屏幕
    glViewport(0, 0, 400, 400); // 设置视口大小为窗口大小
    glClear(GL_COLOR_BUFFER_BIT);

    // 使用边缘检测着色器程序
    glUseProgram(edgeDetectionProgram);

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glUniform1i(glGetUniformLocation(edgeDetectionProgram, "tex"), 0);

    // 定义屏幕空间四边形顶点数据
    GLfloat screenQuadVertices[] = {
       -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
       -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f, 0.0f
    };

    // 启用顶点属性数组
    glEnableVertexAttribArray(edgeDetectionAttributePosition);
    glVertexAttribPointer(edgeDetectionAttributePosition, 3, GL_FLOAT, GL_FALSE, 0, screenQuadVertices);

    // 绘制屏幕空间四边形
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // 禁用顶点属性数组
    glDisableVertexAttribArray(edgeDetectionAttributePosition);

    // 刷新显示
    glutSwapBuffers();
}

void init()
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(400, 400);
    glutCreateWindow("OpenGL Triangle with Edge Detection");

    initGL(); // 初始化GLEW

    initShader(); // 初始化着色器程序

    createFramebuffer(400, 400); // 创建帧缓冲

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    init();

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
