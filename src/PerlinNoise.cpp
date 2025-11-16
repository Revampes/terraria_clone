#include "PerlinNoise.h"

PerlinNoise::PerlinNoise() {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(p.begin(), p.end(), gen);
    p.insert(p.end(), p.begin(), p.end());
}

PerlinNoise::PerlinNoise(unsigned int seed) {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);
    std::mt19937 gen(seed);
    std::shuffle(p.begin(), p.end(), gen);
    p.insert(p.end(), p.begin(), p.end());
}

float PerlinNoise::fade(float t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::lerp(float t, float a, float b) const {
    return a + t * (b - a);
}

float PerlinNoise::grad(int hash, float x, float y) const {
    int h = hash & 3;
    float u = h < 2 ? x : y;
    float v = h < 2 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float PerlinNoise::noise(float x, float y) const {
    int X = static_cast<int>(floor(x)) & 255;
    int Y = static_cast<int>(floor(y)) & 255;
    
    x -= floor(x);
    y -= floor(y);
    
    float u = fade(x);
    float v = fade(y);
    
    int A = p[X] + Y;
    int B = p[X + 1] + Y;
    
    return lerp(v, lerp(u, grad(p[A], x, y), grad(p[B], x - 1, y)),
                lerp(u, grad(p[A + 1], x, y - 1), grad(p[B + 1], x - 1, y - 1)));
}

float PerlinNoise::octaveNoise(float x, float y, int octaves, float persistence, float lacunarity) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; ++i) {
        total += noise(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}
