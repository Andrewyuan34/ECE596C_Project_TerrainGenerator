#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <GL/glew.h>
#include "PerlinNoise.hpp"  // 确保你有一个 PerlinNoise 类来生成噪声

class Terrain {
public:
    Terrain(int width, int height, int step);
    ~Terrain();

    void generateTerrain(double frequency, int octave, double amplitude, double persistence, double lacunarity);

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    std::vector<GLfloat> normals;
    GLuint VAO, VBO, EBO;
    int width, height, step;
    float minHeight, maxHeight;
};

#endif // TERRAIN_H
