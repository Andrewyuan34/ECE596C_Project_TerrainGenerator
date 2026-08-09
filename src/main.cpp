#include <print>
#include <utility>

#include "application.hpp"
#include "cli.hpp"

int main(int argc, char** argv) {
    auto options = tg::parseCommandLine(argc, argv);
    if (!options)
        return options.error();

    const auto& t = options->terrain;
    std::println("Terrain parameters: frequency={} octaves={} amplitude={} "
                 "persistence={} lacunarity={} seed={} width={} lod={}",
                 t.noise.frequency, t.noise.octaves, t.noise.amplitude,
                 t.noise.persistence, t.noise.lacunarity, t.seed, t.width, t.lod);

    tg::Application app{std::move(*options)};
    return app.run();
}
