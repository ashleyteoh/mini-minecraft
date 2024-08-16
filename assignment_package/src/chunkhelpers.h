#pragma once

#include "glm_includes.h"
#include <array>
#include <unordered_map>

//#define BLK_UVX * 0.03125f
//#define BLK_UVY * 0.03125f
//#define BLK_UV 0.03125f


#define BLK_UVX * 0.0625f
#define BLK_UVY * 0.0625f
#define BLK_UV 0.0625f

using namespace std;


enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, SNOW, WATER, LAVA, BEDROCK,

    // tree
    WOOD, LEAF, R_FLOWER, Y_FLOWER,

    // mountain
    R_MUSH, B_MUSH, MUSH_STEM, MUSH_HAT,

    // desert biome
    SAND, SANDSTONE, BUSH, CACTUS,

    //ice biome
    ICE
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

struct VertexData {
    glm::vec4 pos;
    glm::vec4 nor;
    glm::vec4 uv; // x and y are uv coords, z is an animatable flag

    VertexData(glm::vec4 pos, glm::vec4 nor, glm::vec4 uv)
        : pos(pos), nor(nor), uv(uv) {}
};
struct AdjacentFace {
    glm::ivec3 direction;
    std::array<VertexData, 4> vertices;

    AdjacentFace(glm::ivec3 direction, std::array<VertexData, 4> vertices)
        : direction(direction), vertices(vertices) {}
};


// MS2 - UV version - uv x and y coords contain and offset for calculating uv on texture map
const static std::array<AdjacentFace, 6> adjacentBlocks {
    // should start at bottom left corner and go ACW
    //front face
    AdjacentFace(glm::ivec3(1, 0, 0), {VertexData(glm::vec4(1, 0, 0, 1), glm::vec4(1, 0, 0, 0), glm::vec4(BLK_UV, 0, 0, 0)),
                                       VertexData(glm::vec4(1, 1, 0, 1), glm::vec4(1, 0, 0, 0), glm::vec4(BLK_UV, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(1, 1, 1, 1), glm::vec4(1, 0, 0, 0), glm::vec4(0, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(1, 0, 1, 1), glm::vec4(1, 0, 0, 0), glm::vec4(0, 0, 0, 0))}),
    // left face
    AdjacentFace(glm::ivec3(0, 0, 1), {VertexData(glm::vec4(1, 0, 1, 1), glm::vec4(0, 0, 1, 0), glm::vec4(BLK_UV, 0, 0, 0)),
                                       VertexData(glm::vec4(1, 1, 1, 1), glm::vec4(0, 0, 1, 0), glm::vec4(BLK_UV, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(0, 1, 1, 1), glm::vec4(0, 0, 1, 0), glm::vec4(0, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(0, 0, 1, 1), glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 0, 0))}),
    // back face
    AdjacentFace(glm::ivec3(-1, 0, 0), {VertexData(glm::vec4(0, 0, 1, 1), glm::vec4(-1, 0, 0, 0), glm::vec4(BLK_UV, 0, 0, 0)),
                                        VertexData(glm::vec4(0, 1, 1, 1), glm::vec4(-1, 0, 0, 0), glm::vec4(BLK_UV, BLK_UV, 0, 0)),
                                        VertexData(glm::vec4(0, 1, 0, 1), glm::vec4(-1, 0, 0, 0), glm::vec4(0, BLK_UV, 0, 0)),
                                        VertexData(glm::vec4(0, 0, 0, 1), glm::vec4(-1, 0, 0, 0), glm::vec4(0, 0, 0, 0))}),
    // right face
    AdjacentFace(glm::ivec3(0, 0, -1), {VertexData(glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, -1, 0), glm::vec4(BLK_UV, 0, 0, 0)),
                                       VertexData(glm::vec4(0, 1, 0, 1), glm::vec4(0, 0, -1, 0), glm::vec4(BLK_UV, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(1, 1, 0, 1), glm::vec4(0, 0, -1, 0), glm::vec4(0, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(1, 0, 0, 1), glm::vec4(0, 0, -1, 0), glm::vec4(0, 0, 0, 0))}),
    // top face
    AdjacentFace(glm::ivec3(0, 1, 0), {VertexData(glm::vec4(1, 1, 1, 1), glm::vec4(0, 1, 0, 0), glm::vec4(BLK_UV, 0, 0, 0)),
                                       VertexData(glm::vec4(1, 1, 0, 1), glm::vec4(0, 1, 0, 0), glm::vec4(BLK_UV, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(0, 1, 0, 1), glm::vec4(0, 1, 0, 0), glm::vec4(0, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(0, 1, 1, 1), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 0, 0))}),
    // bottom face
    AdjacentFace(glm::ivec3(0, -1, 0), {VertexData(glm::vec4(0, 0, 0, 1), glm::vec4(0, -1, 0, 0), glm::vec4(BLK_UV, 0, 0, 0)),
                                       VertexData(glm::vec4(1, 0, 0, 1), glm::vec4(0, -1, 0, 0), glm::vec4(BLK_UV, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(1, 0, 1, 1), glm::vec4(0, -1, 0, 0), glm::vec4(0, BLK_UV, 0, 0)),
                                       VertexData(glm::vec4(0, 0, 1, 1), glm::vec4(0, -1, 0, 0), glm::vec4(0, 0, 0, 0))})

};

// Grass, dirt, stone, water, snow
const static std::unordered_map<BlockType, std::unordered_map<Direction, glm::vec2, EnumHash>, EnumHash> blockFaces {
    {GRASS, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                               {XNEG, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)},
                                                               {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)}}
    },
    {DIRT, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {XNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)}}
    },
    {STONE, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                               {XNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)}}
    },
    // TODO
    {WATER, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                               {XNEG, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                               {YPOS, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                               {YNEG, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(14.f BLK_UVX, 3.f BLK_UVY)}}
    },
    {SNOW, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                               {XNEG, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                               {YPOS, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                               {YNEG, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)}}
    },
    {ICE, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                               {XNEG, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                               {YPOS, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                               {YNEG, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)}}
    },
    {LAVA, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                              {XNEG, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                              {YPOS, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                              {YNEG, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(14.f BLK_UVX, 1.f BLK_UVY)}}
    },
    {BEDROCK, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                              {XNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)}}
    },


    {WOOD, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                              {XNEG, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YPOS, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YNEG, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)}}
    },
    {LEAF, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                              {XNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                              {YPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                              {YNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)}}
    },

    // VVVV fix to do the cross thing VVVV (bushes & flowers, mushrooms(excluding stem and hat) needs to be fixed!)
    {R_FLOWER, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(12.f BLK_UVX, 15.f BLK_UVY)},
                                                              {XNEG, glm::vec2(12.f BLK_UVX, 15.f BLK_UVY)},
                                                              {YPOS, glm::vec2(12.f BLK_UVX, 15.f BLK_UVY)},
                                                              {YNEG, glm::vec2(12.f BLK_UVX, 15.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(12.f BLK_UVX, 15.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(12.f BLK_UVX, 15.f BLK_UVY)}}
    },
    {Y_FLOWER, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(13.f BLK_UVX, 15.f BLK_UVY)},
                                                              {XNEG, glm::vec2(13.f BLK_UVX, 15.f BLK_UVY)},
                                                              {YPOS, glm::vec2(13.f BLK_UVX, 15.f BLK_UVY)},
                                                              {YNEG, glm::vec2(13.f BLK_UVX, 15.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(13.f BLK_UVX, 15.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(13.f BLK_UVX, 15.f BLK_UVY)}}
    },
    {R_MUSH, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {XNEG, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YPOS, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YNEG, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)}}
    },
    {B_MUSH, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {XNEG, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YPOS, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YNEG, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(13.f BLK_UVX, 14.f BLK_UVY)}}
    },
    {MUSH_STEM, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(13.f BLK_UVX, 7.f BLK_UVY)},
                                                              {XNEG, glm::vec2(13.f BLK_UVX, 7.f BLK_UVY)},
                                                              {YPOS, glm::vec2(14.f BLK_UVX, 7.f BLK_UVY)},
                                                              {YNEG, glm::vec2(14.f BLK_UVX, 7.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(13.f BLK_UVX, 7.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(13.f BLK_UVX, 7.f BLK_UVY)}}
    },
    {MUSH_HAT, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(13.f BLK_UVX, 8.f BLK_UVY)},
                                                              {XNEG, glm::vec2(13.f BLK_UVX, 8.f BLK_UVY)},
                                                              {YPOS, glm::vec2(13.f BLK_UVX, 8.f BLK_UVY)},
                                                              {YNEG, glm::vec2(14.f BLK_UVX, 8.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(13.f BLK_UVX, 8.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(13.f BLK_UVX, 8.f BLK_UVY)}}
    },

    {SAND, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {XNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)}}
    },
    {SANDSTONE, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(0.f BLK_UVX, 3.f BLK_UVY)},
                                                              {XNEG, glm::vec2(0.f BLK_UVX, 3.f BLK_UVY)},
                                                              {YPOS, glm::vec2(0.f BLK_UVX, 4.f BLK_UVY)},
                                                              {YNEG, glm::vec2(0.f BLK_UVX, 2.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(0.f BLK_UVX, 3.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(0.f BLK_UVX, 3.f BLK_UVY)}}
    },

    //fix! VVV
    {BUSH, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(7.f BLK_UVX, 12.f BLK_UVY)},
                                                              {XNEG, glm::vec2(7.f BLK_UVX, 12.f BLK_UVY)},
                                                              {YPOS, glm::vec2(7.f BLK_UVX, 12.f BLK_UVY)},
                                                              {YNEG, glm::vec2(7.f BLK_UVX, 12.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(7.f BLK_UVX, 12.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(7.f BLK_UVX, 12.f BLK_UVY)}}
    },

    {CACTUS, std::unordered_map<Direction, glm::vec2, EnumHash>{{XPOS, glm::vec2(6.f BLK_UVX, 11.f BLK_UVY)},
                                                              {XNEG, glm::vec2(6.f BLK_UVX, 11.f BLK_UVY)},
                                                              {YPOS, glm::vec2(5.f BLK_UVX, 11.f BLK_UVY)},
                                                              {YNEG, glm::vec2(7.f BLK_UVX, 11.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(6.f BLK_UVX, 11.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(6.f BLK_UVX, 11.f BLK_UVY)}}
    }
};

//const static std::unordered_map<glm::ivec3, Direction, EnumHash> dirConvertor {
//    //front face
//    {glm::ivec3(1, 0, 0), XPOS},
//    {glm::ivec3(0, 0, 1), ZPOS},
//    {glm::ivec3(-1, 0, 0), XNEG},
//    {glm::ivec3(0, 0, -1), ZNEG},
//    {glm::ivec3(0, 1, 0), YPOS},
//    {glm::ivec3(0, -1, 0), YNEG},
//};



