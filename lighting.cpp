#include "lighting.hpp"

Lighting::Lighting(float radius, float initialPosY) 
            : radius(radius), lightPosY(initialPosY){}

Lighting::~Lighting() {
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);
    glDeleteVertexArrays(1, &cubeVAO);
}

void Lighting::updateLightPosition(float angle) {
    modelMatrix[12] = radius * cos(angle);
    modelMatrix[14] = radius * sin(angle);
}

void Lighting::initCube() {
    // Cube vertices and indices
    cubeVertices = {
        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f, 
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f
    };
    cubeIndices = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        0, 4, 7,
        7, 3, 0,
        1, 5, 6,
        6, 2, 1,
        3, 2, 6,
        6, 7, 3,
        0, 1, 5,
        5, 4, 0
    };

    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(GLfloat), cubeVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(GLuint), cubeIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

GLfloat Lighting::getmodelMatrix(int i) {
    return modelMatrix[i];
}

GLfloat* Lighting::getmodelMatrix() {
    return modelMatrix;
}

std::vector<GLfloat> Lighting::getVertices() {
    return cubeVertices;
}

std::vector<GLuint> Lighting::getIndices() {
    return cubeIndices;
}

GLuint Lighting::getVAO() {
    return cubeVAO;
}