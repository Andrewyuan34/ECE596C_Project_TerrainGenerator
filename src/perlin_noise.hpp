#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace tg {

// Parameters for fractal Brownian motion (multi-octave noise).
struct FbmParams {
    double frequency   = 3.0;
    int    octaves     = 10;
    double amplitude   = 0.5;
    double persistence = 0.5;
    double lacunarity  = 2.0;
};

// Classic 3D Perlin noise with a seeded permutation table.
// Cheap to construct; copying is allowed (the table is a fixed-size array).
class PerlinNoise {
public:
    explicit PerlinNoise(std::uint32_t seed = 42) noexcept { reseed(seed); }

    void reseed(std::uint32_t seed) noexcept;

    // Raw Perlin noise, output in [0, 1].
    [[nodiscard]] double noise(double x, double y, double z) const noexcept;

    // Multi-octave (fBm) noise; output is scaled to roughly [-maxAmp, maxAmp].
    [[nodiscard]] double fbm(double x, double y, double z,
                             const FbmParams& params) const noexcept;

private:
    static constexpr std::size_t kPermutationSize = 256;

    [[nodiscard]] static constexpr double fade(double t) noexcept {
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }
    [[nodiscard]] static constexpr double lerp(double t, double a, double b) noexcept {
        return a + t * (b - a);
    }
    [[nodiscard]] static double grad(int hash, double x, double y, double z) noexcept;

    std::array<std::uint8_t, 2 * kPermutationSize> p_{};
};

} // namespace tg
