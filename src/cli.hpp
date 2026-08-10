#pragma once

#include <expected>
#include <filesystem>
#include <optional>

#include "terrain_mesh.hpp"

namespace tg {

struct CliOptions {
    TerrainParams terrain{};
    int           windowWidth  = 1280;
    int           windowHeight = 720;
    // Resolved from argv[0], so runtime assets are independent of cwd.
    std::filesystem::path assetRoot;
    // When set: render a single frame into this BMP file and exit without
    // opening a visible window (smoke tests / documentation screenshots).
    std::optional<std::filesystem::path> screenshot;
};

// Parses argv into CliOptions. On parse failure (or after --help) the error
// value is the exit code main() should return.
[[nodiscard]] std::expected<CliOptions, int> parseCommandLine(int argc, char** argv);

} // namespace tg
