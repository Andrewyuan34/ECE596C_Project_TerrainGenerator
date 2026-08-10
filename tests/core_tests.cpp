#include "cli.hpp"
#include "frustum.hpp"
#include "perlin_noise.hpp"
#include "terrain_mesh.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

namespace {

int failures = 0;

void check(bool condition, std::string_view message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::expected<tg::CliOptions, int> parse(std::vector<std::string> args) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (auto& arg : args)
        argv.push_back(arg.data());
    return tg::parseCommandLine(static_cast<int>(argv.size()), argv.data());
}

void testPerlinDeterminism() {
    const tg::PerlinNoise a{1234};
    const tg::PerlinNoise b{1234};
    const tg::PerlinNoise c{4321};
    constexpr double x = 0.173;
    constexpr double y = -0.417;
    constexpr double z = 0.731;
    check(a.noise(x, y, z) == b.noise(x, y, z),
          "equal seeds must produce identical noise");
    check(a.noise(x, y, z) != c.noise(x, y, z),
          "different seeds should produce different noise");
    check(std::isfinite(a.fbm(x, y, z, {})), "fBm must remain finite");
}

void testTerrainInvariants() {
    tg::TerrainParams params;
    params.width = 2;
    params.lod = 0;
    params.seed = 77;

    const tg::TerrainMesh mesh = tg::generateTerrain(params);
    constexpr std::size_t grid = 32;
    constexpr std::size_t expectedIndices = 6 * (grid - 1) * (grid - 1);
    check(mesh.vertices.size() == grid * grid, "LOD 0 vertex count");
    check(mesh.chunks.size() == 1u, "LOD 0 terrain fits in one chunk");
    check(mesh.terrainIndexCount == expectedIndices, "finest chunk index count");
    check(mesh.indices.size() > expectedIndices, "coarser index ranges are generated");
    check(mesh.worldSize == 2048, "width controls world extent");
    check(mesh.minHeight <= mesh.waterLevel && mesh.waterLevel <= mesh.maxHeight,
          "water level must be inside the terrain height range");

    const bool indicesValid = std::ranges::all_of(mesh.indices, [&](std::uint32_t index) {
        return index < mesh.vertices.size();
    });
    check(indicesValid, "all indices must reference existing vertices");

    const bool normalsValid = std::ranges::all_of(mesh.vertices, [](const tg::Vertex& vertex) {
        const float length = glm::length(vertex.normal);
        return std::isfinite(length) && std::abs(length - 1.0f) < 1.0e-4f;
    });
    check(normalsValid, "all normals must be finite unit vectors");

    const tg::TerrainChunk& chunk = mesh.chunks.front();
    check(chunk.lods[0].firstIndex == 0u, "first chunk range starts at zero");
    check(chunk.lods[0].indexCount == expectedIndices, "LOD 0 range size");
    check(chunk.lods[0].indexCount > chunk.lods[1].indexCount &&
          chunk.lods[1].indexCount > chunk.lods[2].indexCount,
          "distance LOD ranges become progressively coarser");
    for (std::size_t lod = 0; lod < tg::kTerrainLodCount; ++lod) {
        const tg::DrawRange range = chunk.lods[lod];
        check(range.firstIndex + range.indexCount <= mesh.indices.size(),
              "chunk draw range stays inside the index buffer");
    }

    const tg::TerrainMesh repeated = tg::generateTerrain(params);
    check(mesh.vertices.front().height == repeated.vertices.front().height,
          "terrain generation must be deterministic");
}

void testTerrainChunking() {
    tg::TerrainParams params;
    params.width = 2;
    params.lod = 1;
    const tg::TerrainMesh mesh = tg::generateTerrain(params);
    check(mesh.chunks.size() == 4u, "64x64 grid splits into four chunks");
    check(mesh.vertices.size() > 64u * 64u, "internal chunk edges receive skirt vertices");

    std::size_t finestIndices = 0u;
    for (const tg::TerrainChunk& chunk : mesh.chunks) {
        check(glm::all(glm::lessThanEqual(chunk.boundsMin, chunk.boundsMax)),
              "chunk AABB is ordered");
        finestIndices += chunk.lods[0].indexCount;
        for (std::size_t lod = 0; lod < tg::kTerrainLodCount; ++lod) {
            const tg::DrawRange range = chunk.lods[lod];
            check(range.firstIndex + range.indexCount <= mesh.indices.size(),
                  "every chunk LOD references a valid index range");
            for (const tg::DrawRange skirt : chunk.skirts[lod])
                check(skirt.firstIndex + skirt.indexCount <= mesh.indices.size(),
                      "every chunk skirt references a valid index range");
        }
    }
    check(mesh.terrainIndexCount == finestIndices,
          "mesh finest-index total matches its chunk ranges");
}

void testFrustumCulling() {
    const glm::mat4 viewProjection =
        glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f) *
        glm::lookAt(glm::vec3{0.0f}, glm::vec3{0.0f, 0.0f, -1.0f},
                    glm::vec3{0.0f, 1.0f, 0.0f});
    const tg::Frustum frustum = tg::Frustum::fromViewProjection(viewProjection);
    check(frustum.intersectsAabb({-0.5f, -0.5f, -3.0f}, {0.5f, 0.5f, -2.0f}),
          "box inside the view frustum is visible");
    check(!frustum.intersectsAabb({20.0f, -0.5f, -3.0f}, {21.0f, 0.5f, -2.0f}),
          "box outside a side plane is culled");
    check(!frustum.intersectsAabb({-0.5f, -0.5f, 2.0f}, {0.5f, 0.5f, 3.0f}),
          "box behind the camera is culled");
}

void testCliValidation() {
    auto valid = parse({"terrain_generator", "--frequency", "4.5", "--lod", "3",
                        "--seed", "99"});
    check(valid.has_value(), "valid CLI options must parse");
    if (valid) {
        check(valid->terrain.noise.frequency == 4.5, "CLI frequency value");
        check(valid->terrain.lod == 3, "CLI LOD value");
        check(valid->terrain.seed == 99, "CLI seed value");
    }

    auto invalid = parse({"terrain_generator", "--lod", "9"});
    check(!invalid.has_value(), "out-of-range CLI options must fail");
}

} // namespace

int main() {
    testPerlinDeterminism();
    testTerrainInvariants();
    testTerrainChunking();
    testFrustumCulling();
    testCliValidation();
    if (failures == 0)
        std::cout << "All core tests passed\n";
    return failures == 0 ? 0 : 1;
}
