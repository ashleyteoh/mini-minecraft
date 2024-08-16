#include "terraingenerator.h"
#include <iostream>

// partially from terrain visualizer instanced.vert.glsl
TerrainGenerator::TerrainGenerator() {}

glm::vec2 TerrainGenerator::random(glm::vec2 p) {
    return glm::normalize(2.f * glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(127.1, 311.7)),
                                                              glm::dot(p, glm::vec2(269.5, 183.3))
                                                              )) * 43758.5453f) - 1.f);
}

glm::vec3 TerrainGenerator::random(glm::vec3 p) {
    glm::vec2 p2 = glm::vec2(p);
    return glm::normalize(2.f * glm::fract(glm::sin(glm::vec3(glm::dot(p2, glm::vec2(127.1, 311.7)),
                                                              glm::dot(p2, glm::vec2(269.5, 183.3)),
                                                              glm::dot(p2, glm::vec2(420.6, 631.2))
                                                              )) * 43758.5453f) - 1.f);
}

template <typename VecType>
float TerrainGenerator::surflet(VecType p, VecType gridPoint) {
    VecType dist = glm::abs(p - gridPoint);
    VecType t = VecType(1.f) - 6.f * glm::pow(dist, VecType(5.f)) + 15.f * glm::pow(dist, VecType(4.f)) - 10.f * glm::pow(dist, VecType(3.f));
    VecType gradient = random(gridPoint);
    VecType diff = p - gridPoint;
    float height = glm::dot(diff, gradient);

    float result = height;
    if (t.length() == 3) {
    }
    for (int i = 0; i < t.length(); ++i) {
        result *= t[i];
    }
    return result;
}

float TerrainGenerator::noise(glm::vec2 uv) {
    const float k = 257.;
    glm::vec4 l  = glm::vec4(glm::floor(uv), glm::fract(uv));
    float u = l.x + l.y * k;
    glm::vec4 v  = glm::vec4(u, u + 1., u + k, u + k + 1.);

    v       = glm::fract(glm::fract(1.23456789f * v) * v / 0.987654321f);
    l.z     = l.z * l.z * (3.f - 2.f * l.z);
    l.w     = l.w * l.w * (3.f - 2.f * l.w);
    l.x     = glm::mix(v.x, v.y, l.z);
    l.y     = glm::mix(v.z, v.w, l.z);

    return    glm::mix(l.x, l.y, l.w);
}

float TerrainGenerator::fbm(glm::vec2 uv) {
    float a = 0.5;
    float f = 5.0;
    float n = 0.;
    int it = 8;
    for (int i = 0; i < 32; i++) {
        if (i < it) {
            n += noise(uv*f)*a;
            a *= 0.5f;
            f *= 2.f;
        }
    }
    return n;
}

float TerrainGenerator::smoothstep(float e1, float e2, float x) {
    float t;
    float temp = (x - e1) / (e2 - e1);
    t = temp < 0 ? 0 : temp > 1 ? 1: temp;
    return t * t * (3.0 - 2.0 * t);
}

float TerrainGenerator::PerlinNoise2D(glm::vec2 uv) {
    glm::vec2 uvXLYL = glm::floor(uv);

    float surfletSum = 0.f;
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            surfletSum += surflet(uv, uvXLYL + glm::vec2(dx, dy));
        }
    }

    return surfletSum;
}

float TerrainGenerator::PerlinNoise3D(glm::vec3 p) {
    glm::vec3 centerP = glm::floor(p);

    float surfletSum = 0.f;
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            for (int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet(p, centerP + glm::vec3(dx, dy, dz));
            }
        }
    }

    return surfletSum;
}

int TerrainGenerator::grassHeight(glm::vec2 xz) {
    float h = 0;

    float amp = 0.5;
    float freq = 128;

    for(int i = 0; i < 4; i++) {
        glm::vec2 offset = glm::vec2(fbm(xz / 256.f), fbm(xz / 300.f) + 1000.f);
        float h1 = abs(PerlinNoise2D((xz + offset * 60.f) / freq));
        h += h1 * amp;

        amp *= 0.5;
        freq *= 0.5;
    }

    // 131 ~ 181 (50 difference)
    h = floor(GROUND_HEIGHT + h * GRASS_DIFF);

    return h;
}

int TerrainGenerator::mountHeight(glm::vec2 xz) {
    float h = 0;

    float amp = 0.7;
    float freq = 80;
    for(int i = 0; i < 4; i++) {
        glm::vec2 offset = glm::vec2(fbm(xz / 256.f), fbm(xz / 300.f) + 1000.f);
        float h1 = abs(PerlinNoise2D((xz + offset * 70.f) / freq));
        h += h1 * amp;

        amp *= 0.5;
        freq *= 0.5;
    }

    h = smoothstep(0.1, 0.8, h) + (0.1 * h);

    // 131 ~ 255 (all the way up to the top)
    h = floor(GROUND_HEIGHT + h * MOUNT_DIFF);

    return h;
}

int TerrainGenerator::desertHeight(glm::vec2 xz) {
    float h = 0;

    float amp = 0.5;
    float freq = 128;

    for(int i = 0; i < 4; i++) {
        glm::vec2 offset = glm::vec2(fbm(xz / 256.f), fbm(xz / 300.f) + 1000.f);
        float h1 = PerlinNoise2D((xz + offset * 60.f) / freq);
        h += h1 * amp;

        amp *= 0.5;
        freq *= 0.5;
    }

    h = smoothstep(0.1, 0.8, h) + (0.1 * h);

    // surface
    h = floor(GROUND_HEIGHT + h * DESERT_DIFF);

    return h;
}

int TerrainGenerator::iceHeight(glm::vec2 xz) {
    float h = 0;

    float amp = 0.8;
    float freq = 32;
    for(int i = 0; i < 4; i++) {
        glm::vec2 offset = glm::vec2(fbm(xz / 256.f), fbm(xz / 300.f) + 1000.f);
        float h1 = abs(PerlinNoise2D((xz + offset * 70.f) / freq));
        h += h1 * amp;

        amp *= 0.5;
        freq *= 0.5;
    }

    //ice peaks
    h = smoothstep(0.4, 0.8, h) + (0.1 * h);

    // water level = GROUND_HEIGHT
    h = floor(GROUND_HEIGHT - 5 + h * ICE_DIFF);

    return h;
}

float TerrainGenerator::blendBiome(glm::vec2 xz) {
    return 0.5 * (PerlinNoise2D(xz / 1024.f) + 1.f);
}

BlockType TerrainGenerator::genCave(glm::vec3 p) {
    glm::vec2 uv = glm::vec2(p.x, p.z);
    glm::vec3 offset = glm::vec3(fbm(uv/256.f), fbm(uv/300.f) + 1500.f, fbm(uv / 382.f));

    BlockType result = STONE; // default stone
    if (PerlinNoise3D((p + offset * 30.f) / 12.f) < 0) {
        result = (p.y < LAVA_TOP) ? LAVA : EMPTY;
    }

    return result;
}

int TerrainGenerator::snowHeight(glm::ivec3 p) {
    float h = 0;
    glm::vec2 xz = glm::vec2(p.x, p.z);
    glm::vec2 offset = glm::vec2(fbm(xz) / 256.f, fbm(xz/300.f) + 1000.f);
    h = abs(PerlinNoise2D(glm::vec2(xz + offset * 30.f) / 10.f));

    h = floor(SNOW_HEIGHT - h * 70);
    return h;
}

int TerrainGenerator::genTreeHeight(glm::ivec3 p) {
    // gen at higher ground than bottom
    float freq = 60.f;
    glm::vec2 xz = glm::vec2(p.x, p.z);
    glm::vec2 offset = glm::vec2(fbm(xz/123.f), fbm(xz/246.f));
    float h = PerlinNoise2D(glm::vec2(xz + offset) / freq);

    if (h > 0) {
        h = 2 + trunc(fbm(glm::vec2(p.x, p.z)) * 10);
        return h;
    }

    return -1;
}

BlockType TerrainGenerator::genFlower(glm::ivec3 p) {
    float freq = 50.f;
    glm::vec2 xz = glm::vec2(p.x, p.z);
    glm::vec2 offset = glm::vec2(fbm(xz/123.f), fbm(xz/246.f));
    float h = PerlinNoise2D(glm::vec2(xz + offset * 20.f) / freq);

    h = trunc(h * 10);
    if (h == 2) { // flower generate zone
        float f = PerlinNoise2D(glm::vec2(xz + offset) / 10.f);
        if (f >= 0.2) return R_FLOWER;
        else if (f < 0.1) return Y_FLOWER;
    }
    return EMPTY; // no flower
}

BlockType TerrainGenerator::genMush(glm::ivec3 p) {
    float freq = 100.f;
    glm::vec2 xz = glm::vec2(p.x, p.z);
    glm::vec2 offset = glm::vec2(fbm(xz/123.f), fbm(xz/246.f));
    float h = PerlinNoise2D(glm::vec2(xz + offset * 20.f) / freq);

    h = trunc(h * 10);
    if (h == 3) { // mush generate zone
        float f = PerlinNoise2D(glm::vec2(xz + offset) / 10.f);
        if (f >= 0.2) return R_MUSH;
        else if (f < 0.1) return B_MUSH;
    }

    return EMPTY;
}

int TerrainGenerator::genPyramid(glm::ivec3 p) {
    float freq = 160.f;
    glm::vec2 xz = glm::vec2(p.x, p.z);
    glm::vec2 offset = glm::vec2(fbm(xz/123.f), fbm(xz/246.f));
    float h = PerlinNoise2D(glm::vec2(xz + offset) / freq);

    h = trunc(h * 10);
    if (h == 3) {
        float f = PerlinNoise2D(glm::vec2(xz + offset) / 10.f);
        return 17 + trunc(fbm(glm::vec2(p.x, p.z)) * 100);
    }

    return -1;
}

int TerrainGenerator::genIgloo(glm::ivec3 p) {
    return 5;
}

std::pair<float, Biome> TerrainGenerator::genBiome(glm::vec2 xz) {
    // gen height for every biome
    int mh = mountHeight(xz);
    int gh = grassHeight(xz);
    int dh = desertHeight(xz);
    int ih = iceHeight(xz);

    float extreme = blendBiome(glm::vec2(xz.y, xz.x) + fbm(xz/234.f));
    extreme = smoothstep(0.3, 0.7, extreme);
    float bumpH = glm::mix(mh, ih, extreme); //mountains - ice
    float flatH = glm::mix(gh, dh, extreme); //grass - desert

    float temperature = blendBiome(xz);
    temperature = smoothstep(0.3, 0.7, temperature);
    float height = glm::mix(bumpH, flatH, temperature);

    // declare which biome
    Biome biome;
    if(temperature < 0.3){
        biome = (extreme < 0.8) ? MOUNTAINS : ICELANDS;
    }
    else {
        biome = (extreme < 0.8) ? GRASSLANDS : DESERT;
    }

    height = min(height, 255.f);
     //test individual biomes
//     height = ih;
//     biome = ICELANDS;

    return std::make_pair(height, biome);
}
