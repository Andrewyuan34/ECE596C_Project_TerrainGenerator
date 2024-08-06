#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <GL/glew.h>
#include "PerlinNoise.hpp"  // 确保你有一个 PerlinNoise 类来生成噪声

class Terrain {
public:
    Terrain(int width, int height, int step);
    ~Terrain();

    void generateBaseTerrain(double frequency, int octave, double amplitude, double persistence, double lacunarity);
    void generateWater();
    void generateTerrainNormals();
    void initTerrain();
    GLuint getVAO() const;
    

    std::vector<GLfloat> vertices, verticesWithNormals;
    std::vector<GLuint> indices;
    GLuint VAO, VBO, EBO;
    int width, height, step;
    float minHeight, maxHeight;
    PerlinNoise perlinNoise(18, 4);
    float waterLevel, HeightDif_low, HeightDif_high, waterdepthMax;
};

#endif // TERRAIN_H
