#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>



struct Vec {
    float x;
    float y;
    float z;

    // Overload += operator for Vec
    Vec& operator+=(const Vec& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this; // Return reference to self for chaining
    }

    // Overload -= operator for Vec
    Vec& operator-=(const Vec& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this; // Return reference to self for chaining
    }

    // Overload * operator for Vec
    Vec operator*(const float scalar) {
        return {x * scalar, y * scalar, z * scalar};
    }

    // Overload + operator for Vec
    Vec operator+(const Vec& other) {
        return {x + other.x, y + other.y, z + other.z};
    }
};



class Camera {
public:
    Camera(Vec pos);

    void keyboard(unsigned char key, int x, int y);
    void mouse(int button, int state, int x, int y);
    void mouseMotion(int x, int y);

    Vec getCameraPos() const;
    Vec getCameraFront() const;

private:
    Vec normalize(Vec vec);
    Vec crossProduct(const Vec vec1, const Vec vec2) const;
    float radians(float degrees) const;

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