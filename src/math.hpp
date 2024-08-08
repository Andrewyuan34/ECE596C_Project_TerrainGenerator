#ifndef MATH_HPP
#define MATH_HPP

#include <cmath>
#include <vector>
#include <GL/glew.h> // Include appropriate OpenGL headers here

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
    Vec operator*(const float scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    // Overload + operator for Vec
    Vec operator+(const Vec& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    // Overload - operator for Vec
    Vec operator-(const Vec& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }
};

// Function to normalize a vector
inline Vec normalize(Vec vec) {
    float length = std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    if (length != 0.0f) {
        vec.x /= length;
        vec.y /= length;
        vec.z /= length;
    }
    return vec;
}

// Function to compute the cross product of two vectors
inline Vec crossProduct(const Vec& vec1, const Vec& vec2) {
    Vec result;
    result.x = vec1.y * vec2.z - vec1.z * vec2.y;
    result.y = vec1.z * vec2.x - vec1.x * vec2.z;
    result.z = vec1.x * vec2.y - vec1.y * vec2.x;
    return result;
}

// Function to convert degrees to radians
inline float radians(float degrees) {
    return degrees * static_cast<float>(M_PI) / 180.0f;
}

// Function to compute the face normal of a triangle
inline Vec computeFaceNormal(const Vec& v1, const Vec& v2, const Vec& v3) {
    Vec edge1 = v2 - v1;
    Vec edge2 = v3 - v1;
    return normalize(crossProduct(edge1, edge2));
}

// Function to compute vertex normals for a mesh
inline void computeVertexNormals(
    const std::vector<GLfloat>& vertices,
    const std::vector<GLuint>& indices,
    std::vector<GLfloat>& normals)
{
    normals.resize(vertices.size(), 0.0f);

    // Traverse all triangles
    for (size_t i = 0; i < indices.size(); i += 3) {
        GLuint index1 = indices[i] * 6; // Assuming each vertex has 6 components (x, y, z, nx, ny, nz)
        GLuint index2 = indices[i + 1] * 6;
        GLuint index3 = indices[i + 2] * 6;

        Vec v1 = {vertices[index1], vertices[index1 + 1], vertices[index1 + 2]};
        Vec v2 = {vertices[index2], vertices[index2 + 1], vertices[index2 + 2]};
        Vec v3 = {vertices[index3], vertices[index3 + 1], vertices[index3 + 2]};

        Vec faceNormal = computeFaceNormal(v1, v3, v2);

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

    // Normalize vertex normals
    for (size_t i = 0; i < normals.size(); i += 6) {
        Vec normal = {normals[i], normals[i + 1], normals[i + 2]};
        normal = normalize(normal);
        normals[i] = normal.x;
        normals[i + 1] = normal.y;
        normals[i + 2] = normal.z;
    }
}

// Function to convert a GLdouble matrix to GLfloat
inline void convertMatrix(GLdouble* source, GLfloat* dest) {
    for (int i = 0; i < 16; ++i) {
        dest[i] = static_cast<GLfloat>(source[i]);
    }
}

#endif // MATH_HPP
