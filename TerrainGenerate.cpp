#include "TerrainGenerate.hpp"
#include <cmath>
#include <limits>
#include <iostream>
#include "PerlinNoise.hpp"
#include "math.hpp"
#include "shader.hpp"

Terrain::Terrain(const int& width, int step, int seed)
    : width(width), height(width), step(width / step), 
    minheight(std::numeric_limits<float>::max()), maxheight(std::numeric_limits<float>::min()), 
    perlinNoise(seed){
        vertices.reserve(width * height * 12 / (step * step));
        indices.reserve(width * height * 12 / (step * step));
        height_map.reserve(width * height / (step * step));
    }

Terrain::~Terrain() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Terrain::generateBaseTerrain(double frequency, int octave, double amplitude, double persistence, double lacunarity) {
    int i = 0;
    //int j = 0;

    std::vector<float> height_array;
    height_array.reserve(width * height / (step * step));

    // 生成地形顶点数据和索引
    for (int z = -height / 2; z < height / 2; z += step) {
        for (int x = -width / 2; x < width / 2; x += step) {
            float nx = static_cast<float>(x) / width;
            float nz = static_cast<float>(z) / height;
            float height = perlinNoise.generateNoise(nx, nz, 0.5, frequency, amplitude, octave, persistence, lacunarity) + 1.5;
            height_array.push_back(height);
            if (height < minheight) minheight = height;
            if (height > maxheight) maxheight = height;
        }
    }

    waterLevel = ((maxheight - minheight) * 0.35f + minheight); // 设置水面高度为地形高度的 40%
    heightDif_low = (minheight + ((maxheight - minheight)* 0.4f)) * width / 60.0f ; // 设置高度差下限
    heightDif_high = heightDif_low * 0.1 ; // 设置高度差上限
    waterdepthMax = (waterLevel - minheight) * width / 60.0f;
    std::cout << minheight * width / 60.0f << " " << maxheight * width / 60.0f << " " << heightDif_low << " " << heightDif_high << std::endl;
    i = 0;
    for (int z = -height / 2; z < height / 2; z += step) {
        for (int x = -width / 2; x < width / 2; x += step) {
            // Adjust the height of the terrain based on the terrain shape
            height_array[i] = perlinNoise.adjustNoiseForTerrainShape(height_array[i], x, z, width, height, step, waterLevel);

            float scaledheight = height_array[i++] * width / 60.0f;
            vertices.push_back(x * 0.1f); // Scale x
            vertices.push_back(scaledheight); // Scale height
            vertices.push_back(z * 0.1f); // Scale z

            // Store the height for later use
            height_map.push_back(scaledheight);
            
            // Generate texture coordinates
            vertices.push_back((static_cast<float>(x) + width / 2) / width);
            vertices.push_back((static_cast<float>(z) + height / 2) / height);

            // Add the height for the terrain, used to determine if the water should be displayed
            vertices.push_back(scaledheight);
        }
    }
    height_array.shrink_to_fit();

    // Generate the terrain indices
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
}

void Terrain::generateWater(){
    int j = 0;
    waterLevel = waterLevel * width / 60.0f;

    // Generate the water plane vertices, following the same pattern as the terrain
    for (int z = -height / 2; z < height / 2; z += step) {
        for (int x = -width / 2; x < width / 2; x += step) {
            vertices.push_back(x * 0.1f); // 宽度缩放
            vertices.push_back(waterLevel); // 固定高度为水平面
            vertices.push_back(z * 0.1f); // 深度缩放

            // 添加纹理坐标
            vertices.push_back((static_cast<float>(x) + width / 2) / width);
            vertices.push_back((static_cast<float>(z) + height / 2) / height);

            // 添加地形高度，用于判断是否显示水面
            vertices.push_back(height_map[j++]);
        }
    }
    height_map.shrink_to_fit();

    // Generate the water plane indices
    int waterOffset = (height / step) * (width / step);
    for (int y = 0; y < height / step - 1; ++y) {
        for (int x = 0; x < width / step - 1; ++x) {
            int start = waterOffset + y * (width / step) + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + width / step + 1);
            indices.push_back(start + width / step + 1);
            indices.push_back(start + width / step);
            indices.push_back(start);
        }
    }
}

void Terrain::generateTerrainNormals(){
    // Calculate the normals for the terrain
    std::vector<GLfloat> normals;
    computeVertexNormals(vertices, indices, normals);

    // Combine the vertices and normals into a single array
    for (size_t i = 0; i < vertices.size() / 6; ++i) {
        verticesWithNormals.push_back(vertices[i * 6]);     // x
        verticesWithNormals.push_back(vertices[i * 6 + 1]); // y
        verticesWithNormals.push_back(vertices[i * 6 + 2]); // z
        verticesWithNormals.push_back(normals[i * 6]);      // nx
        verticesWithNormals.push_back(normals[i * 6 + 1]);  // ny
        verticesWithNormals.push_back(normals[i * 6 + 2]);  // nz
        verticesWithNormals.push_back(vertices[i * 6 + 3]); // u
        verticesWithNormals.push_back(vertices[i * 6 + 4]); // v
        verticesWithNormals.push_back(vertices[i * 6 + 5]); // height
    }
    vertices.shrink_to_fit();
}

void Terrain::initTerrain(const GLuint& shaderProgram){
    // 生成并绑定VAO
    
    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glBindVertexArray(VAO));

    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, verticesWithNormals.size() * sizeof(GLfloat), verticesWithNormals.data(), GL_STATIC_DRAW));
    std::vector<GLfloat>().swap(verticesWithNormals);

    GL_CHECK(glGenBuffers(1, &EBO));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW));
    std::vector<GLuint>().swap(indices);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "aPos");
    GL_CHECK(glEnableVertexAttribArray(posAttrib));
    GL_CHECK(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0));

    GLint normalAttrib = glGetAttribLocation(shaderProgram, "aNormal");
    GL_CHECK(glEnableVertexAttribArray(normalAttrib));
    GL_CHECK(glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))));

    GLint texCoordAttrib = glGetAttribLocation(shaderProgram, "aTexCoord");
    GL_CHECK(glEnableVertexAttribArray(texCoordAttrib));
    GL_CHECK(glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))));

    GLint heightAttrib = glGetAttribLocation(shaderProgram, "aHeight");
    GL_CHECK(glEnableVertexAttribArray(heightAttrib));
    GL_CHECK(glVertexAttribPointer(heightAttrib, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))));
    GL_CHECK(glBindVertexArray(0));
}

const GLuint& Terrain::getVAO() const {
    return VAO;
}

const float& Terrain::getWaterLevel() const {
    return waterLevel;
}

const float& Terrain::getWaterdepthMax() const {
    return waterdepthMax;
}

const float& Terrain::getHeightDif_low() const {
    return heightDif_low;
}

const float& Terrain::getHeightDif_high() const {
    return heightDif_high;
}