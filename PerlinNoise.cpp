#include "PerlinNoise.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <iostream>

const int PerlinNoise::gradientVectors[12][3] = {
    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0},
    {1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0},
    {1, 0, 1}, {-1, 0, 1}, {0, 1, 1}, {0, -1, 1}
};

PerlinNoise::PerlinNoise(int seed, int init_octave) 
                        : octave(init_octave) {
    initialize(seed);
}

void PerlinNoise::initialize(int seed) {
    p.resize(permutationTableSize);
    std::iota(p.begin(), p.end(), 0);
    std::mt19937 mt(seed);
    std::shuffle(p.begin(), p.end(), mt);
    p.insert(p.end(), p.begin(), p.end());
}

double PerlinNoise::noise(double x, double y, double z) const {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;
    int Z = (int)std::floor(z) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = p[X] + Y;
	int AA = p[A] + Z;
	int AB = p[A + 1] + Z;
	int B = p[X + 1] + Y;
	int BA = p[B] + Z;
	int BB = p[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x-1, y, z)), lerp(u, grad(p[AB], x, y-1, z), grad(p[BB], x-1, y-1, z))),	lerp(v, lerp(u, grad(p[AA+1], x, y, z-1), grad(p[BA+1], x-1, y, z-1)), lerp(u, grad(p[AB+1], x, y-1, z-1),	grad(p[BB+1], x-1, y-1, z-1))));
	return (res + 1.0)/2.0;
}

double PerlinNoise::generateNoise(double x, double y, double z, double frequency, double amplitude, int octave
                                    , double persistence, double lacunarity) {
    double noiseValue = 0.0;
    //double frequency = 2.0;
    //double amplitude = 0.6;
    //double persistence = 0.5;  // Adjust persistence (typically in the range of 0.4 to 0.6)
    //double lacunarity = 2.0;   // Adjust lacunarity (typically around 2.0)
    double maxAmplitude = 0.0;

    for (int i = 0; i < octave; ++i) {
        noiseValue += amplitude * noise(x * frequency, y * frequency, z * frequency);

        // Update maxAmplitude for scaling the final noise value
        maxAmplitude += amplitude;

        // Update frequency for the next octave and amplitude using lacunarity and persistence
        frequency *= lacunarity;
        amplitude *= persistence;
    }
    //std:: cout << "maxAmplitude: " << maxAmplitude << "amplitude: " << amplitude << "octave: " << octave << "\n";
    //std:: cout << "amplitude * octave / 2: " << amplitude * octave / 2 << "\n";
    // Mapping the result to the range of -1 to 1
    noiseValue /= maxAmplitude;
    noiseValue = 2.0 * noiseValue - 1.0;

    return noiseValue * maxAmplitude;
}

double PerlinNoise::adjustNoiseForTerrainShape(double noiseValue, double x, double z, double width, double height, int step, double waterLevel) {
    // for the boundary points, the height is always set to above the water level 
    if (x == width / 2 - step || z == height / 2 - step || x == - width / 2 || z == - width / 2) {
        if(noiseValue < waterLevel) {
            noiseValue = (waterLevel - noiseValue) * 0.2 + waterLevel;
            return noiseValue;
        } else {
            return noiseValue;            
        }
    }else return noiseValue;
}



double PerlinNoise::fade(double t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b) const {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) const {
    int h = hash & 11;
    double u = gradientVectors[h][0];
    double v = gradientVectors[h][1];
    double w = gradientVectors[h][2];
    return u * x + v * y + w * z;
}

