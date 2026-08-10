#include "terrain_mesh.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace tg {

namespace {
constexpr int   kBaseWorldSize = 1024;  // worldSize = kBaseWorldSize * width
constexpr float kHeightScale   = 1.0f / 60.0f;
constexpr float kXZScale       = 0.1f;
constexpr std::size_t kChunkCells = 32;

void appendSurfaceQuad(std::vector<std::uint32_t>& indices,
                       std::uint32_t topLeft, std::uint32_t topRight,
                       std::uint32_t bottomLeft, std::uint32_t bottomRight) {
    indices.insert(indices.end(), {
        topLeft, topRight, bottomRight,
        bottomRight, bottomLeft, topLeft,
    });
}

void appendDoubleSidedSkirtQuad(std::vector<std::uint32_t>& indices,
                                std::uint32_t topA, std::uint32_t topB,
                                std::uint32_t skirtA, std::uint32_t skirtB) {
    indices.insert(indices.end(), {
        topA, skirtA, skirtB, skirtB, topB, topA,
        topA, topB, skirtB, skirtB, skirtA, topA,
    });
}
} // namespace

TerrainMesh generateTerrain(const TerrainParams& params) {
    TerrainMesh mesh;

    const int worldSize = kBaseWorldSize * params.width;
    const int step      = worldSize / (32 * (1 << params.lod));
    const int gridN     = worldSize / step;  // vertices per side
    const float scale   = worldSize * kHeightScale;

    const PerlinNoise noise{params.seed};

    // 1) Sample the height field (raw noise space). A flat vector keeps this
    //    portable to standard libraries that do not yet ship std::mdspan.
    const std::size_t n = static_cast<std::size_t>(gridN);
    std::vector<float> heights(n * n);
    const auto hmap = [&heights, n](std::size_t row, std::size_t column) -> float& {
        return heights[row * n + column];
    };

    float minH = std::numeric_limits<float>::max();
    float maxH = std::numeric_limits<float>::lowest();

    for (std::size_t j = 0; j < static_cast<std::size_t>(gridN); ++j) {
        const int z = -worldSize / 2 + static_cast<int>(j) * step;
        for (std::size_t i = 0; i < static_cast<std::size_t>(gridN); ++i) {
            const int x = -worldSize / 2 + static_cast<int>(i) * step;
            const double nx = static_cast<double>(x) / worldSize;
            const double nz = static_cast<double>(z) / worldSize;
            const float  h  = static_cast<float>(noise.fbm(nx, nz, 0.5, params.noise) + 1.5);
            hmap(j, i) = h;
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
    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < n; ++i) {
            const bool boundary = (i == 0 || j == 0 || i == n - 1 || j == n - 1);
            float& h = hmap(j, i);
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
        return glm::normalize(glm::vec3{hmap(j, il) - hmap(j, ir),
                                        2.0f * cell,
                                        hmap(jt, i) - hmap(jb, i)});
    };

    // 5) Terrain vertices.
    mesh.vertices.reserve(n * n);
    for (std::size_t j = 0; j < n; ++j) {
        const float z = (-worldSize / 2 + static_cast<int>(j) * step) * kXZScale;
        for (std::size_t i = 0; i < n; ++i) {
            const float x = (-worldSize / 2 + static_cast<int>(i) * step) * kXZScale;
            const float h = hmap(j, i);
            mesh.vertices.push_back(Vertex{
                .position = {x, h, z},
                .normal   = gridNormal(j, i),
                .uv       = {static_cast<float>(i) / static_cast<float>(n),
                             static_cast<float>(j) / static_cast<float>(n)},
                .height   = h,
            });
        }
    }

    // 6) Split the grid into independently cullable chunks. Each chunk stores
    //    three index-only LODs (strides 1, 2 and 4) over the same vertices.
    //    Internal edges receive downward skirts so adjacent LODs cannot expose
    //    cracks where their edge tessellation differs.
    const std::size_t cellCount = n - 1u;
    const auto vertexIndex = [n](std::size_t row, std::size_t column) {
        return static_cast<std::uint32_t>(row * n + column);
    };

    for (std::size_t z0 = 0; z0 < cellCount; z0 += kChunkCells) {
        const std::size_t z1 = std::min(z0 + kChunkCells, cellCount);
        for (std::size_t x0 = 0; x0 < cellCount; x0 += kChunkCells) {
            const std::size_t x1 = std::min(x0 + kChunkCells, cellCount);
            TerrainChunk chunk;

            float chunkMinY = std::numeric_limits<float>::max();
            float chunkMaxY = std::numeric_limits<float>::lowest();
            for (std::size_t z = z0; z <= z1; ++z) {
                for (std::size_t x = x0; x <= x1; ++x) {
                    chunkMinY = std::min(chunkMinY, hmap(z, x));
                    chunkMaxY = std::max(chunkMaxY, hmap(z, x));
                }
            }

            const auto worldCoordinate = [worldSize, step](std::size_t coordinate) {
                return (-worldSize / 2 + static_cast<int>(coordinate) * step) * kXZScale;
            };
            chunk.boundsMin = {worldCoordinate(x0),
                               std::min(chunkMinY, mesh.waterLevel),
                               worldCoordinate(z0)};
            chunk.boundsMax = {worldCoordinate(x1),
                               std::max(chunkMaxY, mesh.waterLevel),
                               worldCoordinate(z1)};

            const auto edgeSkirtDepth = [&](const std::vector<std::uint32_t>& edge) {
                float maximumError = 0.0f;
                for (std::size_t lod = 1u; lod < kTerrainLodCount; ++lod) {
                    const std::size_t stride = std::size_t{1} << lod;
                    for (std::size_t a = 0; a + 1u < edge.size(); a += stride) {
                        const std::size_t b = std::min(a + stride, edge.size() - 1u);
                        const float heightA = mesh.vertices[edge[a]].position.y;
                        const float heightB = mesh.vertices[edge[b]].position.y;
                        for (std::size_t point = a + 1u; point < b; ++point) {
                            const float t = static_cast<float>(point - a) /
                                            static_cast<float>(b - a);
                            const float chordHeight = heightA + t * (heightB - heightA);
                            const float actualHeight = mesh.vertices[edge[point]].position.y;
                            maximumError = std::max(maximumError,
                                                    std::abs(actualHeight - chordHeight));
                        }
                    }
                }
                return maximumError + 0.05f;
            };
            float lowestSkirtY = chunk.boundsMin.y;
            const auto makeSkirt = [&](const std::vector<std::uint32_t>& edge) {
                std::vector<std::uint32_t> skirt;
                skirt.reserve(edge.size());
                const float depth = edgeSkirtDepth(edge);
                for (const std::uint32_t index : edge) {
                    Vertex vertex = mesh.vertices[index];
                    vertex.position.y -= depth;
                    lowestSkirtY = std::min(lowestSkirtY, vertex.position.y);
                    skirt.push_back(static_cast<std::uint32_t>(mesh.vertices.size()));
                    mesh.vertices.push_back(vertex);
                }
                return skirt;
            };
            const auto makeHorizontalEdge = [&](std::size_t z) {
                std::vector<std::uint32_t> edge;
                edge.reserve(x1 - x0 + 1u);
                for (std::size_t x = x0; x <= x1; ++x)
                    edge.push_back(vertexIndex(z, x));
                return edge;
            };
            const auto makeVerticalEdge = [&](std::size_t x) {
                std::vector<std::uint32_t> edge;
                edge.reserve(z1 - z0 + 1u);
                for (std::size_t z = z0; z <= z1; ++z)
                    edge.push_back(vertexIndex(z, x));
                return edge;
            };

            std::vector<std::uint32_t> top, topSkirt;
            std::vector<std::uint32_t> bottom, bottomSkirt;
            std::vector<std::uint32_t> left, leftSkirt;
            std::vector<std::uint32_t> right, rightSkirt;
            if (z0 > 0u) {
                top = makeHorizontalEdge(z0);
                topSkirt = makeSkirt(top);
            }
            if (z1 < cellCount) {
                bottom = makeHorizontalEdge(z1);
                bottomSkirt = makeSkirt(bottom);
            }
            if (x0 > 0u) {
                left = makeVerticalEdge(x0);
                leftSkirt = makeSkirt(left);
            }
            if (x1 < cellCount) {
                right = makeVerticalEdge(x1);
                rightSkirt = makeSkirt(right);
            }
            chunk.boundsMin.y = lowestSkirtY;

            const auto appendSkirtEdge = [&](DrawRange& range,
                                             const std::vector<std::uint32_t>& edge,
                                             const std::vector<std::uint32_t>& skirt,
                                             std::size_t stride) {
                range.firstIndex = mesh.indices.size();
                if (edge.empty()) {
                    range.indexCount = 0u;
                    return;
                }
                for (std::size_t a = 0; a + 1u < edge.size(); a += stride) {
                    const std::size_t b = std::min(a + stride, edge.size() - 1u);
                    appendDoubleSidedSkirtQuad(mesh.indices,
                                               edge[a], edge[b], skirt[a], skirt[b]);
                }
                range.indexCount = mesh.indices.size() - range.firstIndex;
            };

            for (std::size_t lod = 0; lod < kTerrainLodCount; ++lod) {
                const std::size_t stride = std::size_t{1} << lod;
                DrawRange& range = chunk.lods[lod];
                range.firstIndex = mesh.indices.size();
                for (std::size_t z = z0; z < z1; z += stride) {
                    const std::size_t nextZ = std::min(z + stride, z1);
                    for (std::size_t x = x0; x < x1; x += stride) {
                        const std::size_t nextX = std::min(x + stride, x1);
                        appendSurfaceQuad(mesh.indices,
                                          vertexIndex(z, x), vertexIndex(z, nextX),
                                          vertexIndex(nextZ, x), vertexIndex(nextZ, nextX));
                    }
                }
                range.indexCount = mesh.indices.size() - range.firstIndex;
                appendSkirtEdge(chunk.skirts[lod][0], top, topSkirt, stride);
                appendSkirtEdge(chunk.skirts[lod][1], bottom, bottomSkirt, stride);
                appendSkirtEdge(chunk.skirts[lod][2], left, leftSkirt, stride);
                appendSkirtEdge(chunk.skirts[lod][3], right, rightSkirt, stride);
            }
            mesh.terrainIndexCount += chunk.lods.front().indexCount;
            mesh.chunks.push_back(std::move(chunk));
        }
    }

    const std::size_t chunksPerAxis = (cellCount + kChunkCells - 1u) / kChunkCells;
    for (std::size_t row = 0; row < chunksPerAxis; ++row) {
        for (std::size_t column = 0; column < chunksPerAxis; ++column) {
            TerrainChunk& chunk = mesh.chunks[row * chunksPerAxis + column];
            if (row > 0u)
                chunk.neighbors[0] = static_cast<std::int32_t>((row - 1u) * chunksPerAxis + column);
            if (row + 1u < chunksPerAxis)
                chunk.neighbors[1] = static_cast<std::int32_t>((row + 1u) * chunksPerAxis + column);
            if (column > 0u)
                chunk.neighbors[2] = static_cast<std::int32_t>(row * chunksPerAxis + column - 1u);
            if (column + 1u < chunksPerAxis)
                chunk.neighbors[3] = static_cast<std::int32_t>(row * chunksPerAxis + column + 1u);
        }
    }

    return mesh;
}

} // namespace tg
