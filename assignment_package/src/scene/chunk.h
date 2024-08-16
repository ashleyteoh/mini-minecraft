#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "drawable.h"
#include "chunkHelpers.h"

//using namespace std;

const static std::array<glm::vec2, 4> adjacentBlocks2d {
    // thinking of x as the horizontal axis
    // right face
    glm::vec2(1, 0), glm::vec2(0, -1), glm::vec2(-1, 0), glm::vec2(0, 1)
};

class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    // The coordinates of the chunk's lower-left corner in world space
    int minX, minZ;
    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    Direction dirConvertor(glm::ivec3 direction);
    bool isTrns(BlockType t);
    void bufferDataTrns(std::vector<VertexData> data, std::vector<GLuint> idx);
    void bufferDataOpq(std::vector<VertexData> data, std::vector<GLuint> idx);
    //bool areNeighboursInMap();
    //void bufferDataInterleaved(std::vector<VertexData> data, std::vector<GLuint> idx);
    bool checkGenFace(BlockType baseType, BlockType neighbourType);

public:
    //Chunk(int x, int z);
    BlockType getLocalBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getLocalBlockAt(int x, int y, int z) const;
    void setLocalBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);

    Chunk(OpenGLContext *context, int x, int z);
    void createVBOdata() override;
    glm::vec3 colourFromBlockType(BlockType);
    // checks if any neighbours are empty and if so, returns the direction
    //glm::ivec3 hasEmptyNeighbour()

    bool isInChunk(glm::ivec3 pos);
    friend class Terrain;
    friend class ShaderProgram;

    //static void buildVBODataForChunk(Chunk *chunk, ChunkVBOData c);

    void bufferVBOtoGPU();

//    std::vector<VertexData> data;
//    std::vector<GLuint> idx;

    std::vector<VertexData> opqData;
    std::vector<VertexData> trnsData;
    std::vector<GLuint> opqIdx;
    std::vector<GLuint> trnsIdx;

};

