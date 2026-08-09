#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "perlin_noise.hpp"

namespace tg {

// Everything needed to generate one terrain. Derived from the CLI options.
struct TerrainParams {
    FbmParams     noise{};
    std::uint32_t seed  = 42;
    int           width = 6;  // width multiplier, range [1, 13]
    int           lod   = 1;  // level of detail, range [0, 5]
};

// Interleaved vertex layout shared by the terrain and the water plane:
// position (3) | normal (3) | uv (2) | world-space terrain height (1).
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    float     height;
};

static_assert(sizeof(Vertex) == 9 * sizeof(float),
              "Vertex must stay tightly packed for glVertexAttribPointer");

// Pure CPU-side mesh data. No OpenGL types appear here; the renderer owns
// the GPU upload. This keeps mesh generation independent of the render API.
struct TerrainMesh {
    std::vector<Vertex>        vertices;
    std::vector<std::uint32_t> indices;

    // indices[0, terrainIndexCount) draws the terrain;
    // indices[terrainIndexCount, indices.size()) draws the water plane.
    std::size_t terrainIndexCount = 0;

    float waterLevel    = 0.0f;  // world-space Y of the water surface
    float minHeight     = 0.0f;
    float maxHeight     = 0.0f;
    float heightDifLow  = 0.0f;  // texture blending range (see terrain.frag)
    float heightDifHigh = 0.0f;
    float waterDepthMax = 0.0f;
    int   worldSize     = 0;     // world-space extent of the square terrain
};

[[nodiscard]] TerrainMesh generateTerrain(const TerrainParams& params);

} // namespace tg
