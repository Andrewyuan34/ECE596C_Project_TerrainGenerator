#include "perlin_noise.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

namespace tg {

namespace {
// The 12 edge-midpoint gradient vectors of a cube (Ken Perlin's set).
// Keeping the set symmetric avoids introducing a preferred axis into the
// generated terrain.
constexpr std::array<std::array<int, 3>, 12> kGradientVectors{{
    { 1,  1,  0}, {-1,  1,  0}, { 1, -1,  0}, {-1, -1,  0},
    { 1,  0,  1}, {-1,  0,  1}, { 1,  0, -1}, {-1,  0, -1},
    { 0,  1,  1}, { 0, -1,  1}, { 0,  1, -1}, { 0, -1, -1},
}};
} // namespace

void PerlinNoise::reseed(std::uint32_t seed) noexcept {
    std::array<std::uint8_t, kPermutationSize> perm{};
    std::iota(perm.begin(), perm.end(), std::uint8_t{0});
    std::mt19937 engine{seed};
    std::shuffle(perm.begin(), perm.end(), engine);
    // Duplicate the table to avoid index wrapping in noise().
    std::copy(perm.begin(), perm.end(), p_.begin());
    std::copy(perm.begin(), perm.end(), p_.begin() + kPermutationSize);
}

double PerlinNoise::noise(double x, double y, double z) const noexcept {
    const int X = static_cast<int>(std::floor(x)) & 255;
    const int Y = static_cast<int>(std::floor(y)) & 255;
    const int Z = static_cast<int>(std::floor(z)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    const double u = fade(x);
    const double v = fade(y);
    const double w = fade(z);

    // Hash coordinates of the 8 cube corners.
    const int A  = p_[X] + Y;
    const int AA = p_[A] + Z;
    const int AB = p_[A + 1] + Z;
    const int B  = p_[X + 1] + Y;
    const int BA = p_[B] + Z;
    const int BB = p_[B + 1] + Z;

    // Trilinear blend of the 8 corner gradients.
    const double res =
        lerp(w, lerp(v, lerp(u, grad(p_[AA], x, y, z),
                                grad(p_[BA], x - 1, y, z)),
                        lerp(u, grad(p_[AB], x, y - 1, z),
                                grad(p_[BB], x - 1, y - 1, z))),
                lerp(v, lerp(u, grad(p_[AA + 1], x, y, z - 1),
                                grad(p_[BA + 1], x - 1, y, z - 1)),
                        lerp(u, grad(p_[AB + 1], x, y - 1, z - 1),
                                grad(p_[BB + 1], x - 1, y - 1, z - 1))));
    return (res + 1.0) / 2.0;
}

double PerlinNoise::fbm(double x, double y, double z,
                        const FbmParams& params) const noexcept {
    double value = 0.0;
    double maxAmplitude = 0.0;
    double frequency = params.frequency;
    double amplitude = params.amplitude;

    for (int i = 0; i < params.octaves; ++i) {
        value += amplitude * noise(x * frequency, y * frequency, z * frequency);
        maxAmplitude += amplitude;
        frequency *= params.lacunarity;
        amplitude *= params.persistence;
    }

    // Normalize to [-1, 1], then rescale by the accumulated amplitude
    // (kept identical to the original project's mapping).
    value /= maxAmplitude;
    value = 2.0 * value - 1.0;
    return value * maxAmplitude;
}

double PerlinNoise::grad(int hash, double x, double y, double z) noexcept {
    const auto& g = kGradientVectors[static_cast<std::size_t>(hash) %
                                     kGradientVectors.size()];
    return g[0] * x + g[1] * y + g[2] * z;
}

} // namespace tg
