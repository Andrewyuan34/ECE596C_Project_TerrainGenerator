#include "TerrainGenerate.hpp"
#include <cmath>
#include <limits>
#include <iostream>
#include "PerlinNoise.hpp"

Terrain::Terrain(int width, int height, int step)
    : width(width), height(height), step(step), minHeight(std::numeric_limits<float>::max()), maxHeight(std::numeric_limits<float>::min()) {
        vertices.reserve(width * height * 12 / (step * step));
        indices.reserve(width * height * 12 / (step * step));
    }

Terrain::~Terrain() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Terrain::generateBaseTerrain(double frequency, int octave, double amplitude, double persistence, double lacunarity) {
    std::vector<float> height_map(WIDTH * HEIGHT / (step * step));
    int i = 0;
    int j = 0;

    std::vector<float> height_array;
    height_array.reserve(WIDTH * HEIGHT / (step * step));

    // 生成地形顶点数据和索引
    for (int z = -HEIGHT / 2; z < HEIGHT / 2; z += step) {
        for (int x = -WIDTH / 2; x < WIDTH / 2; x += step) {
            float nx = static_cast<float>(x) / WIDTH;
            float ny = static_cast<float>(z) / HEIGHT;
            float height = perlinNoise.generateNoise(nx, ny, 0.5, frequency, amplitude, octave, persistence, lacunarity) + 1.5;
            height_array.push_back(height);
            if (height < minHeight) minHeight = height;
            if (height > maxHeight) maxHeight = height;
        }
    }

    waterLevel = ((maxHeight - minHeight) * 0.45f + minHeight); // 设置水面高度为地形高度的 40%
    HeightDif_low = (minHeight + ((maxHeight - minHeight)* 0.5f)) * WIDTH / 60.0f ; // 设置高度差下限
    HeightDif_high = (minHeight + ((maxHeight - minHeight)* 0.01f)) * WIDTH / 60.0f ; // 设置高度差上限
    waterdepthMax = (waterLevel - minHeight) * WIDTH / 60.0f;

    i = 0;
    for (int z = -HEIGHT / 2; z < HEIGHT / 2; z += step) {
        for (int x = -WIDTH / 2; x < WIDTH / 2; x += step) {
            // Adjust the height of the terrain based on the terrain shape
            height_array[i] = perlinNoise.adjustNoiseForTerrainShape(height_array[i], x, z, WIDTH, HEIGHT, step, waterLevel);

            float scaledHeight = height_array[i++] * WIDTH / 60.0f;
            vertices.push_back(x * 0.1f); // Scale x
            vertices.push_back(scaledHeight); // Scale height
            vertices.push_back(z * 0.1f); // Scale z

            // Store the height for later use
            height_map[j++] = scaledHeight;
            
            // Generate texture coordinates
            vertices.push_back(2 * (static_cast<float>(x) + WIDTH / 2) / WIDTH);
            vertices.push_back(2 * (static_cast<float>(z) + HEIGHT / 2) / HEIGHT);

            // Add the height for the terrain, used to determine if the water should be displayed
            vertices.push_back(scaledHeight);
        }
        
    }

    // Generate the terrain indices
    for (int y = 0; y < HEIGHT / step - 1; ++y) {
        for (int x = 0; x < WIDTH / step - 1; ++x) {
            int start = y * (WIDTH / step) + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step);
            indices.push_back(start);
        }
    }
}

void Terrain::generateWater(){
    int j = 0;
    waterLevel = waterLevel * WIDTH / 60.0f;

    // Generate the water plane vertices, following the same pattern as the terrain
    for (int z = -HEIGHT / 2; z < HEIGHT / 2; z += step) {
        for (int x = -WIDTH / 2; x < WIDTH / 2; x += step) {
            vertices.push_back(x * 0.1f); // 宽度缩放
            vertices.push_back(waterLevel); // 固定高度为水平面
            vertices.push_back(z * 0.1f); // 深度缩放

            // 添加纹理坐标
            vertices.push_back((static_cast<float>(x) + WIDTH / 2) / WIDTH);
            vertices.push_back((static_cast<float>(z) + HEIGHT / 2) / HEIGHT);

            // 添加地形高度，用于判断是否显示水面
            vertices.push_back(height_map[j++]);
        }
    }

    // Generate the water plane indices
    int waterOffset = (HEIGHT / step) * (WIDTH / step);
    for (int y = 0; y < HEIGHT / step - 1; ++y) {
        for (int x = 0; x < WIDTH / step - 1; ++x) {
            int start = waterOffset + y * (WIDTH / step) + x;
            indices.push_back(start);
            indices.push_back(start + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step + 1);
            indices.push_back(start + WIDTH / step);
            indices.push_back(start);
        }
    }
}

void Terrain::generateTerrainNormals(){
    // Calculate the normals for the terrain
    std::vector<GLfloat> normals;
    computeVertexNormals(vertices, indices, normals);

    // Combine the vertices and normals into a single array
    std::vector<GLfloat> verticesWithNormals;
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
}

void initTerrain(){
    // 生成并绑定VAO
    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glBindVertexArray(VAO));

    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, verticesWithNormals.size() * sizeof(GLfloat), verticesWithNormals.data(), GL_STATIC_DRAW));

    GL_CHECK(glGenBuffers(1, &EBO));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW));

    GLint posAttrib = glGetAttribLocation(shaderProgram1, "aPos");
    GL_CHECK(glEnableVertexAttribArray(posAttrib));
    GL_CHECK(glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)0));

    GLint normalAttrib = glGetAttribLocation(shaderProgram1, "aNormal");
    GL_CHECK(glEnableVertexAttribArray(normalAttrib));
    GL_CHECK(glVertexAttribPointer(normalAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))));

    GLint texCoordAttrib = glGetAttribLocation(shaderProgram1, "aTexCoord");
    GL_CHECK(glEnableVertexAttribArray(texCoordAttrib));
    GL_CHECK(glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))));

    GLint heightAttrib = glGetAttribLocation(shaderProgram1, "aHeight");
    GL_CHECK(glEnableVertexAttribArray(heightAttrib));
    GL_CHECK(glVertexAttribPointer(heightAttrib, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))));

    GL_CHECK(glBindVertexArray(0));
}

GLuint Terrain::getVAO() const {
    return VAO;
}