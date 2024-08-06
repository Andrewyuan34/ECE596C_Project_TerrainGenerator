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

