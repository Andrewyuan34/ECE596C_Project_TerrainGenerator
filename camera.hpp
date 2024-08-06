#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>
#include <vector>
#include <GL/glew.h>
#include "math.hpp"

class Camera {
public:
    Camera(Vec pos);

    void keyboard(unsigned char key, int x, int y);
    void mouse(int button, int state, int x, int y);
    void mouseMotion(int x, int y);

    Vec getCameraPos() const;
    Vec getCameraFront() const;

private:
    Vec cameraPos;
    Vec cameraFront;
    Vec cameraUp;

    float mouseSpeed;
    bool middleButtonPressed;
    float lastX, lastY;
    bool firstMouse;
    float yaw, pitch;
    const float moveSpeed = 0.07f;
};

#endif // CAMERA_H