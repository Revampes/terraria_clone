#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>

class PerlinNoise {
public:
    PerlinNoise();
    PerlinNoise(unsigned int seed);
    
    float noise(float x, float y) const;
    float octaveNoise(float x, float y, int octaves, float persistence, float lacunarity) const;
    
private:
    std::vector<int> p;
    float fade(float t) const;
    float lerp(float t, float a, float b) const;
    float grad(int hash, float x, float y) const;
};

#endif // PERLINNOISE_H
