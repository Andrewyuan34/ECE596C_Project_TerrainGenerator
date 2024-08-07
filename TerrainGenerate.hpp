#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <GL/glew.h>
#include "PerlinNoise.hpp"  

class Terrain {
public:
    Terrain(const int& width, int step, int seed);
    ~Terrain();

    void generateBaseTerrain(double frequency, int octave, double amplitude, double persistence, double lacunarity);
    void generateWater();
    void generateTerrainNormals();
    void initTerrain(const GLuint& shaderProgram);
    const GLuint& getVAO() const;
    const float& getWaterLevel() const;
    const float& getWaterdepthMax() const;
    const float& getHeightDif_low() const;
    const float& getHeightDif_high() const;

    std::vector<GLfloat> vertices, verticesWithNormals;
    std::vector<GLuint> indices;
    GLuint VAO, VBO, EBO;
    int width, height, step;
    float minheight, maxheight;
    PerlinNoise perlinNoise;
    float waterLevel, heightDif_low, heightDif_high, waterdepthMax;
    std::vector<float> height_map;
};

#endif // TERRAIN_H
