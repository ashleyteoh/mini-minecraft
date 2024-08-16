#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#include "glm_includes.h"
#include "scene/chunk.h"

#define GROUND_HEIGHT 131
#define GRASS_DIFF 50
#define MOUNT_DIFF 110
#define DESERT_DIFF 70
#define ICE_DIFF 80
#define LAVA_TOP 25
#define SNOW_HEIGHT 200

enum Biome : unsigned char
{
    MOUNTAINS, GRASSLANDS, DESERT, ICELANDS
};

class TerrainGenerator
{
public:
    TerrainGenerator();

    // Noise helper functions
    static glm::vec2 random(glm::vec2 p);
    static glm::vec3 random(glm::vec3 p);

    template <typename VecType>
    static float surflet(VecType p, VecType gridPoint);

    static float noise(glm::vec2 uv);
    static float fbm(glm::vec2 uv);
    static float smoothstep(float e1, float e2, float x);

    // Perlin Noise
    static float PerlinNoise2D(glm::vec2 uv);
    static float PerlinNoise3D(glm::vec3 p);


    // **** terrain generation **** //
    // height functions for terrains
    static int grassHeight(glm::vec2 xz);
    static int mountHeight(glm::vec2 xz);
    static int desertHeight(glm::vec2 xz);
    static int iceHeight(glm::vec2 xz);

    // blend biome function
    static float blendBiome(glm::vec2 xz);

    // final height function to be called
    // returns a pair of height and biome of that block
    static std::pair<float, Biome> genBiome(glm::vec2 xz);

    // generates cave, returns LAVA, EMPTY or STONE
    static BlockType genCave(glm::vec3 p);

    // generates int(height) where snow will start
    static int snowHeight(glm::ivec3 p);


    // **** feature generation **** //
    // tree height for regular and mushroom trees
    static int genTreeHeight(glm::ivec3 p);

    // empty, red, yellow
    static BlockType genFlower(glm::ivec3 p);

    // empty, red, brown
    static BlockType genMush(glm::ivec3 p);

    // returns pyramid base's side length
    static int genPyramid(glm::ivec3 p);

    // igloo
    static int genIgloo(glm::ivec3 p);
};

#endif // TERRAINGENERATOR_H
