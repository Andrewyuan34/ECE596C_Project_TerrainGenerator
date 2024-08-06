#include "TerrainGenerate.hpp"
#include <cmath>
#include <limits>
#include <iostream>

Terrain::Terrain(int width, int height, int step)
    : width(width), height(height), step(step), minHeight(std::numeric_limits<float>::max()), maxHeight(std::numeric_limits<float>::min()) {
    setupTerrain();
}

Terrain::~Terrain() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Terrain::generateTerrain(double frequency, int octave, double amplitude, double persistence, double lacunarity) {
    PerlinNoise perlinNoise;
    vertices.clear();
    indices.clear();

    std::vector<float> heightMap(width * height);

    for (int z = -height / 2; z < height / 2; z += step) {
        for (int x = -width / 2; x < width / 2; x += step) {
            float nx = static_cast<float>(x) / width;
            float ny = static_cast<float>(z) / height;
            float height = perlinNoise.generateNoise(nx, ny, 0.5, frequency, amplitude, octave, persistence, lacunarity) + 1.5;
            vertices.push_back(x);
            vertices.push_back(height);
            vertices.push_back(z);

            if (height < minHeight) minHeight = height;
            if (height > maxHeight) maxHeight = height;
        }
    }

    // Generate indices
    for (int y = 0; y < height / step - 1; ++y) {
        for (int x = 0; x < width / step - 1; ++x) {
            int start = y * (width / step) + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + width / step + 1);
            indices.push_back(start + width / step + 1);
            indices.push_back(start + width / step);
            indices.push_back(start);
        }
    }

    computeNormals();
    bufferData();
}
