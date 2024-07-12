#ifndef PERLINNOISE_HPP
#define PERLINNOISE_HPP

#include <vector>

class PerlinNoise {
public:
    PerlinNoise(int seed = 0, int init_octave = 10); //sadsadsadasdasddasdas

    double noise(double x, double y , double z) const;

    double generateNoise(double x, double y, double z, double frequency = 2.0, double amplitude = 0.6, int octave = 10
                                    , double persistence = 0.5, double lacunarity = 2.0);

    void setOctave(int newOctave) { octave = newOctave; }
    int getOctave() const { return octave; }

private:
    void initialize(int seed);

    double fade(double t) const;
    double lerp(double t, double a, double b) const;
    double grad(int hash, double x, double y, double z) const;

    std::vector<int> p; // Permutation table
    static const int permutationTableSize = 256; // Size of permutation table
    static const int gradientVectors[12][3]; // Predefined gradient vectors
    int octave;
};

#endif // PERLINNOISE_HPP
