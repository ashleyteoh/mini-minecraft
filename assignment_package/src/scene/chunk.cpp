#include "chunk.h"
#include <iostream>
#include "glm/ext.hpp"

#if 0
Chunk::Chunk(int x, int z) : m_blocks(), minX(x), minZ(z), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}
#endif

Chunk::Chunk(OpenGLContext *context, int x, int z) : Drawable(context), m_blocks(), minX(x), minZ(z), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getLocalBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getLocalBlockAt(int x, int y, int z) const {
    return getLocalBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setLocalBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

glm::vec3 Chunk::colourFromBlockType(BlockType t) {
    switch(t) {
    case GRASS:
        return glm::vec3(95.f, 159.f, 53.f) / 255.f;
        //break;
    case DIRT:
        return glm::vec3(121.f, 85.f, 58.f) / 255.f;
        //break;
    case STONE:
        return glm::vec3(0.5f);
        //break;
    case WATER:
        return glm::vec3(0.f, 0.f, 0.75f);
        //break;
    case SNOW:
        return glm::vec3(0.f, 0.f, 0.f);
    default:
        // Other block types are not yet handled, so we default to debug purple
        return glm::vec3(1.f, 0.f, 1.f);
        //break;
    }
}


// MS2 version - creating two separate buffers for opaque and translucent blocks
void Chunk::createVBOdata() {
    opqData.clear();
    trnsData.clear();
    opqIdx.clear();
    trnsData.clear();

//    std::vector<VertexData> opqData;
//    std::vector<VertexData> trnsData;
//    std::vector<GLuint> opqIdx;
//    std::vector<GLuint> trnsIdx;
    unsigned int opqNumFaces = 0;
    unsigned int trnsNumFaces = 0;


    // Iterate over every block in this chunk
    for(int z = 0; z < 16; ++z) {
        for(int y = 0; y < 256; ++y) {
            for(int x = 0; x < 16; ++x) {
                BlockType currentBlock = getLocalBlockAt(x, y, z);
                // Look at 6 blocks surrounding currentBlock
                // If that neighbour is EMPTY, add a face for currentBlock in that direction
                std::vector<VertexData> *data = nullptr;
                std::vector<GLuint> *idx = nullptr;
                unsigned int *numFaces = nullptr;

                if (currentBlock == EMPTY) continue;
                if (isTrns(currentBlock)) {
                    data = &trnsData;
                    idx = &trnsIdx;
                    numFaces = &trnsNumFaces;
                } else {
                    data = &opqData;
                    idx = &opqIdx;
                    numFaces = &opqNumFaces;
                }
                for(auto &neighbour : adjacentBlocks) {
                   // neighbour position in local coords
                   glm::ivec3 neighbourPos = glm::ivec3(x, y, z) + neighbour.direction;

                   // check if neighbour is in bounds of chunk
                    bool genFace = false;

                    if(isInChunk(neighbourPos) && checkGenFace(currentBlock,
                                                               getLocalBlockAt(neighbourPos.x, neighbourPos.y, neighbourPos.z))) {
                        genFace = true;
                    } else if (!isInChunk(neighbourPos)) {
                        // neighbour falls outside given chunk - check neighbouring chunks
                        Chunk* neighbourChunk = nullptr;

                        Direction neighbourDir = dirConvertor(neighbour.direction);
                        neighbourChunk = this->m_neighbors[neighbourDir];

                        glm::ivec3 localNeighbourPos = glm::ivec3((neighbourPos.x + 16) % 16, neighbourPos.y, (neighbourPos.z + 16) % 16);

                        if (neighbourChunk && checkGenFace(currentBlock,
                                                           neighbourChunk->getLocalBlockAt(localNeighbourPos.x, neighbourPos.y, localNeighbourPos.z))) {
                            genFace = true;
                        }
                    }
                    if (genFace) {
                        for(VertexData vert : neighbour.vertices) {
                            vert.pos += glm::vec4(x + minX, y, z + minZ, 0);

                            Direction dir = dirConvertor(neighbour.direction);
                            glm::vec2 blockUV = blockFaces.at(currentBlock).at(dir);

                            vert.uv += glm::vec4(blockUV, 0, 0);

                            if(currentBlock == WATER || currentBlock == LAVA) {
                                // add animatable flag
                                vert.uv += glm::vec4(0, 0, 1, 0);
                            }
                            data->push_back(vert);
                        }
                        // make indices here
                        for (int i : {0, 1, 2, 0, 2, 3}) {
                            idx->push_back(*numFaces * 4 + i);
                        }
                        if (isTrns(currentBlock)) {
                            ++trnsNumFaces;
                        } else {
                            ++opqNumFaces;
                        }
                    }
                }
            }
        }
    }

    //bufferDataTrns(trnsData, trnsIdx);
    //bufferDataOpq(opqData, opqIdx);
}


void Chunk::bufferDataTrns(std::vector<VertexData> data, std::vector<GLuint> idx) {
    // TODO generate buffer, bind buffer, glVertexDAta -
    generateBuffer(IDX_TRNS);
    bindBuffer(IDX_TRNS);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    // initialise count variable in drawable
    indexCounts[IDX_TRNS] = idx.size();

    generateBuffer(INTERLEAVED_TRNS);
    bindBuffer(INTERLEAVED_TRNS);
    mp_context->glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(VertexData), data.data(), GL_STATIC_DRAW);
}


void Chunk::bufferDataOpq(std::vector<VertexData> data, std::vector<GLuint> idx) {
    generateBuffer(IDX_OPQ);
    bindBuffer(IDX_OPQ);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_STATIC_DRAW);

    // initialise count variable in drawable
    indexCounts[IDX_OPQ] = idx.size();

    generateBuffer(INTERLEAVED_OPQ);
    bindBuffer(INTERLEAVED_OPQ);
    mp_context->glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(VertexData), data.data(), GL_STATIC_DRAW);
}


bool Chunk::isInChunk(glm::ivec3 pos) {
    return (pos.x >= 0 && pos.x < 16) && (pos.y >= 0 && pos.y < 256) && (pos.z >= 0 && pos.z < 16);
}

Direction Chunk::dirConvertor(glm::ivec3 direction) {
    if (direction == glm::ivec3(1, 0, 0)) {
        return XPOS;
    } else if (direction == glm::ivec3(-1, 0, 0)) {
        return XNEG;
    } else if (direction == glm::ivec3(0, 0, 1)) {
        return ZPOS;
    } else if (direction == glm::ivec3(0, 0, -1)) {
        return ZNEG;
    } else if (direction == glm::ivec3(0, 1, 0)) {
        return YPOS;
    } else if (direction == glm::ivec3(0, -1, 0)) {
        return YNEG;
    }
};

bool Chunk::isTrns(BlockType t) {
    return t == WATER || t == ICE || t == R_FLOWER || t == Y_FLOWER;
}

bool Chunk::checkGenFace(BlockType baseType, BlockType neighbourType) {
    if (baseType != WATER) {
        return neighbourType == EMPTY || isTrns(neighbourType);
    } else {
        return neighbourType == EMPTY;
    }
}




