#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
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

// Interleaved vertex layout shared by the terrain and the water pass:
// position (3) | normal (3) | uv (2) | world-space terrain height (1).
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    float     height;
};

static_assert(sizeof(Vertex) == 9 * sizeof(float),
              "Vertex must stay tightly packed for glVertexAttribPointer");

inline constexpr std::size_t kTerrainLodCount = 3;
inline constexpr std::size_t kTerrainChunkEdgeCount = 4;

struct DrawRange {
    std::size_t firstIndex = 0;
    std::size_t indexCount = 0;
};

// A chunk owns no vertices; it references the shared height-field vertex
// buffer through three progressively coarser index ranges.
struct TerrainChunk {
    glm::vec3 boundsMin{0.0f};
    glm::vec3 boundsMax{0.0f};
    std::array<DrawRange, kTerrainLodCount> lods{};
    // Edge order: north (-Z), south (+Z), west (-X), east (+X).
    std::array<std::array<DrawRange, kTerrainChunkEdgeCount>,
               kTerrainLodCount> skirts{};
    std::array<std::int32_t, kTerrainChunkEdgeCount> neighbors{-1, -1, -1, -1};
};

// Pure CPU-side mesh data. No OpenGL types appear here; the renderer owns
// the GPU upload. This keeps mesh generation independent of the render API.
struct TerrainMesh {
    std::vector<Vertex>        vertices;
    std::vector<std::uint32_t> indices;
    std::vector<TerrainChunk>  chunks;

    // Total number of indices across every chunk's finest range. The same
    // chunk ranges draw terrain and water; water is lifted in the shader.
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
