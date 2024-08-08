#include "camera.hpp"
#include <GL/glut.h>
#include <iostream>

Camera::Camera(Vec pos)
    : cameraPos(pos), cameraFront(Vec{0, 0, -1}), cameraUp(Vec{0, 1, 0}), mouseSpeed(0.001f),
      middleButtonPressed(false), lastX(400), lastY(300), firstMouse(true),
      yaw(-90.0f), pitch(0.0f) {}

void Camera::keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
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
    case '1': // Toggle wireframe mode
        showWireframe = !showWireframe;
        break;
    }
    glutPostRedisplay();
}

void Camera::mouse(int button, int state, int x, int y)
{
    if (button == 3)
    { // Scroll up
        // Move camera forward along cameraFront direction
        cameraPos += normalize(cameraFront) * moveSpeed * 100;
    }
    else if (button == 4)
    { // Scroll down
        // Move camera backward along cameraFront direction
        cameraPos -= normalize(cameraFront) * moveSpeed * 100;
    }
    else if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            middleButtonPressed = true;    // Set the middle button state to pressed
            lastX = static_cast<float>(x); // Save the last x position of the mouse
            lastY = static_cast<float>(y); // Save the last y position of the mouse
        }
        else
        {
            middleButtonPressed = false;
        }
    }
    glutPostRedisplay();
}

void Camera::mouseMotion(int x, int y)
{
    if (middleButtonPressed)
    {
        float sensitivity = 0.05f; // Adjust this value to change the mouse sensitivity

        float xpos = static_cast<float>(x);
        float ypos = static_cast<float>(y);

        if (firstMouse)
        { // Avoid camera jump when the mouse first moves
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        // Calculate offset from last mouse position
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
        lastX = xpos;
        lastY = ypos;

        // Apply sensitivity to the offset
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        // Update yaw and pitch angles based on mouse movement
        yaw += xoffset;
        pitch += yoffset;

        // Restrict the camera's pitch to avoid gimbal lock
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // Calculate the new cameraFront vector
        Vec front;
        front.x = std::cos(radians(yaw)) * std::cos(radians(pitch));
        front.y = std::sin(radians(pitch));
        front.z = std::sin(radians(yaw)) * std::cos(radians(pitch));
        cameraFront = normalize(front);

        glutPostRedisplay();
    }
}

Vec Camera::getCameraPos() const
{
    return cameraPos;
}

Vec Camera::getCameraFront() const
{
    return cameraFront;
}

Vec Camera::getCameraUp() const
{
    return cameraUp;
}

const bool &Camera::getShowWireframe() const
{
    return showWireframe;
}


