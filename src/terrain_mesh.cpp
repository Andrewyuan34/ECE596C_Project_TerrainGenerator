#include "terrain_mesh.hpp"

#include <algorithm>
#include <limits>
#include <mdspan>

namespace tg {

namespace {
constexpr int   kBaseWorldSize = 1024;  // worldSize = kBaseWorldSize * width
constexpr float kHeightScale   = 1.0f / 60.0f;
constexpr float kXZScale       = 0.1f;
} // namespace

TerrainMesh generateTerrain(const TerrainParams& params) {
    TerrainMesh mesh;

    const int worldSize = kBaseWorldSize * params.width;
    const int step      = worldSize / (32 * (1 << params.lod));
    const int gridN     = worldSize / step;  // vertices per side
    const float scale   = worldSize * kHeightScale;

    const PerlinNoise noise{params.seed};

    // 1) Sample the height field (raw noise space), tracked via mdspan.
    std::vector<float> heights(static_cast<std::size_t>(gridN) * gridN);
    auto hmap = std::mdspan<float, std::dextents<std::size_t, 2>>(
        heights.data(), gridN, gridN);

    float minH = std::numeric_limits<float>::max();
    float maxH = std::numeric_limits<float>::lowest();

    for (std::size_t j = 0; j < static_cast<std::size_t>(gridN); ++j) {
        const int z = -worldSize / 2 + static_cast<int>(j) * step;
        for (std::size_t i = 0; i < static_cast<std::size_t>(gridN); ++i) {
            const int x = -worldSize / 2 + static_cast<int>(i) * step;
            const double nx = static_cast<double>(x) / worldSize;
            const double nz = static_cast<double>(z) / worldSize;
            const float  h  = static_cast<float>(noise.fbm(nx, nz, 0.5, params.noise) + 1.5);
            hmap[j, i] = h;
            minH = std::min(minH, h);
            maxH = std::max(maxH, h);
        }
    }

    // 2) Water level & shader thresholds (same formulas as the original).
    const float waterLevelRaw = minH + 0.35f * (maxH - minH);
    mesh.minHeight     = minH * scale;
    mesh.maxHeight     = maxH * scale;
    mesh.waterLevel    = waterLevelRaw * scale;
    mesh.heightDifLow  = (minH + 0.4f * (maxH - minH)) * scale;
    mesh.heightDifHigh = mesh.heightDifLow * 0.1f;
    mesh.waterDepthMax = (waterLevelRaw - minH) * scale;
    mesh.worldSize     = worldSize;

    // 3) Keep island edges above the water level, then convert heights to
    //    world space.
    const std::size_t n = static_cast<std::size_t>(gridN);
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < n; ++i) {
            const bool boundary = (i == 0 || j == 0 || i == n - 1 || j == n - 1);
            float& h = hmap[j, i];
            if (boundary && h < waterLevelRaw)
                h = (waterLevelRaw - h) * 0.2f + waterLevelRaw;
            h *= scale;
        }
    }

    // 4) Smooth normals via central differences on the scaled height grid.
    const float cell = step * kXZScale;
    const auto gridNormal = [&](std::size_t j, std::size_t i) {
        const std::size_t il = (i > 0) ? i - 1 : i;
        const std::size_t ir = (i < n - 1) ? i + 1 : i;
        const std::size_t jt = (j > 0) ? j - 1 : j;
        const std::size_t jb = (j < n - 1) ? j + 1 : j;
        return glm::normalize(glm::vec3{hmap[j, il] - hmap[j, ir],
                                        2.0f * cell,
                                        hmap[jt, i] - hmap[jb, i]});
    };

    // 5) Terrain vertices.
    mesh.vertices.reserve(n * n);
    for (std::size_t j = 0; j < n; ++j) {
        const float z = (-worldSize / 2 + static_cast<int>(j) * step) * kXZScale;
        for (std::size_t i = 0; i < n; ++i) {
            const float x = (-worldSize / 2 + static_cast<int>(i) * step) * kXZScale;
            const float h = hmap[j, i];
            mesh.vertices.push_back(Vertex{
                .position = {x, h, z},
                .normal   = gridNormal(j, i),
                .uv       = {static_cast<float>(i) / static_cast<float>(n),
                             static_cast<float>(j) / static_cast<float>(n)},
                .height   = h,
            });
        }
    }

    // 6) A single index grid is reused by the terrain and water passes. The
    //    vertex shader lifts the existing positions to the water level.
    for (std::uint32_t j = 0; j + 1 < n; ++j) {
        for (std::uint32_t i = 0; i + 1 < n; ++i) {
            const std::uint32_t start = j * static_cast<std::uint32_t>(n) + i;
            for (std::uint32_t idx : {start, start + 1, start + static_cast<std::uint32_t>(n) + 1,
                                      start + static_cast<std::uint32_t>(n) + 1,
                                      start + static_cast<std::uint32_t>(n), start}) {
                mesh.indices.push_back(idx);
            }
        }
    }
    mesh.terrainIndexCount = mesh.indices.size();

    return mesh;
}

} // namespace tg
