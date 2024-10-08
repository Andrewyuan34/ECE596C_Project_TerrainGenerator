#ifndef LIGHTING_H
#define LIGHTING_H

#include <GL/glew.h>
#include <vector>
#include <cmath>

class Lighting {
public:
    Lighting();
    ~Lighting();

    void init(float radius, float initialPosY);
    void updateLightPosition(float angle);
    void initCube(const int& factor);
    GLfloat getmodelMatrix(int i);
    GLfloat* getmodelMatrix();
    std::vector<GLfloat> getVertices();
    std::vector<GLuint> getIndices();
    GLuint getVAO();


    GLuint cubeVAO, cubeVBO, cubeEBO;
    float radius;
    float lightPosY;
    std::vector<GLfloat> cubeVertices;
    std::vector<GLuint> cubeIndices;
    GLfloat modelMatrix[16] = {
        5.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 5.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 5.0f, 0.0f,
        radius, lightPosY, 0.0f, 1.0f
    };
};

#endif // LIGHTING_H