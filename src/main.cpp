#include <iostream>
#include <utility>

#include "application.hpp"
#include "cli.hpp"

int main(int argc, char** argv) {
    auto options = tg::parseCommandLine(argc, argv);
    if (!options)
        return options.error();

    const auto& t = options->terrain;
    std::cout << "Terrain parameters: frequency=" << t.noise.frequency
              << " octaves=" << t.noise.octaves
              << " amplitude=" << t.noise.amplitude
              << " persistence=" << t.noise.persistence
              << " lacunarity=" << t.noise.lacunarity
              << " seed=" << t.seed
              << " width=" << t.width
              << " lod=" << t.lod << '\n';

    tg::Application app{std::move(*options)};
    return app.run();
}
