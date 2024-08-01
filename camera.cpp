#include "camera.hpp"
#include <GL/glut.h>
#include <iostream>

Camera::Camera(Vec pos)
    : cameraPos(pos), cameraFront(Vec{0, 0, -1}), cameraUp(Vec{0, 1, 0}), mouseSpeed(0.001f),
      middleButtonPressed(false), lastX(400), lastY(300), firstMouse(true),
      yaw(-90.0f), pitch(0.0f) {}

void Camera::keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': // Move camera forward horizontally
            cameraPos += normalize(crossProduct(cameraUp, crossProduct(cameraFront, cameraUp))) * moveSpeed * 100;
            break;
        case 's': // Move camera backward
            cameraPos -= normalize(crossProduct(cameraUp, crossProduct(cameraFront, cameraUp))) * moveSpeed * 100;
            break;
        case 'a': // Move camera left
            cameraPos -= normalize(crossProduct(cameraFront, cameraUp)) * moveSpeed * 100;
            break;
        case 'd': // Move camera right
            cameraPos += normalize(crossProduct(cameraFront, cameraUp)) * moveSpeed * 100;
            break;
        case 'r': // Move camera up vertically
            cameraPos += cameraUp * moveSpeed * 100;
            break;
        case 'f': // Move camera down vertically
            cameraPos -= cameraUp * moveSpeed * 100;
            break;
    }
    glutPostRedisplay();
}

void Camera::mouse(int button, int state, int x, int y) {
    if (button == 3) { // Scroll up
        // Move camera forward along cameraFront direction
        cameraPos += normalize(cameraFront) * moveSpeed * 100;
    } else if (button == 4) { // Scroll down
        // Move camera backward along cameraFront direction
        cameraPos -= normalize(cameraFront) * moveSpeed * 100;
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

void Camera::mouseMotion(int x, int y) {
    if (middleButtonPressed) {
        float sensitivity = 0.03f; // 调整这个值以改变灵敏度

        float xpos = static_cast<float>(x);
        float ypos = static_cast<float>(y);

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            std::cout << "firstMouse" << std::endl;
            firstMouse = false;
        }

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

        // 计算新的相机前进方向
        Vec front;
        front.x = std::cos(radians(yaw)) * std::cos(radians(pitch));
        front.y = std::sin(radians(pitch));
        front.z = std::sin(radians(yaw)) * std::cos(radians(pitch));
        cameraFront = normalize(front);

        glutPostRedisplay();
    }
}

Vec Camera::getCameraPos() const {
    return cameraPos;
}

Vec Camera::getCameraFront() const {
    return cameraFront;
}

Vec normalize(Vec vec) {
    float length = std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    if (length != 0.0f) {
        vec.x /= length;
        vec.y /= length;
        vec.z /= length;
    }
    return vec;
}

Vec crossProduct(const Vec vec1, const Vec vec2)  {
    Vec result;
    result.x = vec1.y * vec2.z - vec1.z * vec2.y;
    result.y = vec1.z * vec2.x - vec1.x * vec2.z;
    result.z = vec1.x * vec2.y - vec1.y * vec2.x;
    return result;
}

float radians(float degrees)  {
    return degrees * static_cast<float>(M_PI) / 180.0f;
}

// 计算法线的辅助函数
Vec computeFaceNormal(const Vec &v1, const Vec &v2, const Vec &v3) {
    Vec edge1 = v2 - v1;
    Vec edge2 = v3 - v1;
    return normalize(crossProduct(edge1, edge2));
}

void computeVertexNormals(
    const std::vector<GLfloat> &vertices, 
    const std::vector<GLuint> &indices, 
    std::vector<GLfloat> &normals) 
{
    normals.resize(vertices.size(), 0.0f);

    // 遍历所有三角形
    for (size_t i = 0; i < indices.size(); i += 3) {
        GLuint index1 = indices[i] * 6;
        GLuint index2 = indices[i + 1] * 6;
        GLuint index3 = indices[i + 2] * 6;

        Vec v1 = {vertices[index1], vertices[index1 + 1], vertices[index1 + 2]};
        Vec v2 = {vertices[index2], vertices[index2 + 1], vertices[index2 + 2]};
        Vec v3 = {vertices[index3], vertices[index3 + 1], vertices[index3 + 2]};

        Vec faceNormal = computeFaceNormal(v1, v2, v3);

        normals[index1] += faceNormal.x;
        normals[index1 + 1] += faceNormal.y;
        normals[index1 + 2] += faceNormal.z;

        normals[index2] += faceNormal.x;
        normals[index2 + 1] += faceNormal.y;
        normals[index2 + 2] += faceNormal.z;

        normals[index3] += faceNormal.x;
        normals[index3 + 1] += faceNormal.y;
        normals[index3 + 2] += faceNormal.z;
    }

    // 归一化顶点法线
    for (size_t i = 0; i < normals.size(); i += 6) {
        Vec normal = {normals[i], normals[i + 1], normals[i + 2]};
        normal = normalize(normal);
        normals[i] = -normal.x;
        normals[i + 1] = -normal.y;
        normals[i + 2] = -normal.z;
    }
}