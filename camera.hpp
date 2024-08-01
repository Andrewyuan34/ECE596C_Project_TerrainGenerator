#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>
#include <vector>
#include <GL/glew.h>

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
    Vec operator*(const float scalar) const{
        return {x * scalar, y * scalar, z * scalar};
    }

    // Overload + operator for Vec
    Vec operator+(const Vec& other) const{
        return {x + other.x, y + other.y, z + other.z};
    }

    Vec operator-(const Vec& other) const{
        return {x - other.x, y - other.y, z - other.z};
    }
};

Vec normalize(Vec vec);
Vec crossProduct(const Vec vec1, const Vec vec2);
float radians(float degrees);
Vec computeFaceNormal(const Vec &v1, const Vec &v2, const Vec &v3);
void computeVertexNormals(const std::vector<GLfloat> &vertices, const std::vector<GLuint> &indices, std::vector<GLfloat> &normals);



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