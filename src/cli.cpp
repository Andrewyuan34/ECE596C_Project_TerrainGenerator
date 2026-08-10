#include "cli.hpp"

#include <CLI/CLI.hpp>

namespace tg {

std::expected<CliOptions, int> parseCommandLine(int argc, char** argv) {
    CliOptions opts;
    std::error_code pathError;
    const auto executable = std::filesystem::weakly_canonical(
        std::filesystem::absolute(argv[0], pathError), pathError);
    opts.assetRoot = pathError ? std::filesystem::current_path()
                               : executable.parent_path();
    CLI::App app{"Procedural terrain generator (Perlin noise + OpenGL 3.3 Core)"};
    app.set_help_flag("-h,--help", "Print this help message and exit");

    auto& noise = opts.terrain.noise;
    app.add_option("-f,--frequency", noise.frequency, "Set frequency")
        ->check(CLI::Range(1.0, 5.0));
    app.add_option("-o,--octave", noise.octaves, "Set octave count")
        ->check(CLI::Range(2, 20));
    app.add_option("-a,--amplitude", noise.amplitude, "Set amplitude")
        ->check(CLI::Range(0.4, 0.8));
    app.add_option("-p,--persistence", noise.persistence, "Set persistence")
        ->check(CLI::Range(0.4, 0.6));
    app.add_option("-l,--lacunarity", noise.lacunarity, "Set lacunarity")
        ->check(CLI::Range(1.0, 3.0));
    app.add_option("-w,--width", opts.terrain.width, "Set width multiplier")
        ->check(CLI::Range(1, 13));
    app.add_option("-d,--lod", opts.terrain.lod, "Set level of detail")
        ->check(CLI::Range(0, 5));
    app.add_option("-s,--seed", opts.terrain.seed, "Set random seed");
    app.add_option("--window-width", opts.windowWidth, "Window width in pixels")
        ->check(CLI::Range(320, 7680));
    app.add_option("--window-height", opts.windowHeight, "Window height in pixels")
        ->check(CLI::Range(240, 4320));
    app.add_option("--screenshot", opts.screenshot,
                   "Render one frame to a BMP file and exit (hidden window)");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        // app.exit() prints the error/help text and yields the exit code.
        return std::unexpected(app.exit(e));
    }
    return opts;
}

} // namespace tg
