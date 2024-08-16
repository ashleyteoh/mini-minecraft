#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>

#define CHUNK_WIDTH 16
#define WATER_TABLE 135
#define STONE_TOP 128
#define STONE_BOTTOM 1 //change this to 1 after multithreading!!!

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(),
    m_chunkVBOsNeedUpdating(true), mp_context(context), generator(TerrainGenerator())
{}

Terrain::~Terrain() {
    //m_geomCube.destroyVBOdata();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getGlobalBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getLocalBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                                  static_cast<unsigned int>(y),
                                  static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getGlobalBlockAt(glm::vec3 p) const {
    return getGlobalBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setGlobalBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setLocalBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                           static_cast<unsigned int>(y),
                           static_cast<unsigned int>(z - chunkOrigin.y),
                           t);
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    //uPtr<Chunk> chunk = mkU<Chunk>(x, z);
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);

    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    //bottom left coord for keying chunks

    // check if haschunkat, if not instantiate then draw
    // change parameters - give player pos

    //glm::ivec2 curr_chunk = glm::ivec2(16 * glm::floor(playerPos.x / 16.f), 16 * glm::floor(playerPos.x / 16.f));

//    int x_min =
//            ...
//                hasChunkAt(i, j) ?
//                    c = getChunkAt(i, j)
//                    ProceduralHelp(c);
//                    ...->draw(c*)


    // mygl - expand - createvbo
    // mygl - update - draw
    /*
    if(m_chunkVBOsNeedUpdating) {
        m_geomCube.clearOffsetBuf();
        m_geomCube.clearColorBuf();

        std::vector<glm::vec3> offsets, colors;

        for(int x = minX; x < maxX; x += 16) {
            for(int z = minZ; z < maxZ; z += 16) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                for(int i = 0; i < 16; ++i) {
                    for(int j = 0; j < 256; ++j) {
                        for(int k = 0; k < 16; ++k) {
                            BlockType t = chunk->getLocalBlockAt(i, j, k);

                            if(t != EMPTY) {
                                offsets.push_back(glm::vec3(i+x, j, k+z));
                                switch(t) {
                                case GRASS:
                                    colors.push_back(glm::vec3(95.f, 159.f, 53.f) / 255.f);
                                    break;
                                case DIRT:
                                    colors.push_back(glm::vec3(121.f, 85.f, 58.f) / 255.f);
                                    break;
                                case STONE:
                                    colors.push_back(glm::vec3(0.5f));
                                    break;
                                case WATER:
                                    colors.push_back(glm::vec3(0.f, 0.f, 0.75f));
                                    break;
                                default:
                                    // Other block types are not yet handled, so we default to debug purple
                                    colors.push_back(glm::vec3(1.f, 0.f, 1.f));
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        m_geomCube.createInstancedVBOdata(offsets, colors); //all instanced calls should be a To-Do!
        m_chunkVBOsNeedUpdating = false;
    }
    */

//    shaderProgram->drawInstanced(m_geomCube);
//    shaderProgram->draw2(..)
}

void Terrain::draw(glm::vec2 playerPos, ShaderProgram *shaderProgram) {
    //bottom left coord for keying chunks
    // .y is actually the z coord, assuming the vec2 passed to the function contains (x-coord, z-coord)
    chunksWithBlockDataMutex.lock();
    for (auto &chunkPair : m_chunks) {
        uPtr<Chunk> &chunk = chunkPair.second;
        if(chunk->elemCount(IDX_OPQ) < 0 || chunk->elemCount(IDX_TRNS) < 0) {
            continue;
        }

        //shaderProgram->drawInterleaved(*chunk);
        shaderProgram->drawTranslucent(*chunk);
        shaderProgram->drawOpaque(*chunk);
        // if (hasChunkAt(chunk->minX, chunk->minZ)) {
        //     uPtr<Chunk>& currChunk = getChunkAt(currChunkX, currChunkZ);
        //     currChunk->createVBOdata();
        //     shaderProgram->drawInterleaved(*chunk);
        // }

    }
    chunksWithBlockDataMutex.unlock();

}

void Terrain::CreateTestScene()
{
    // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!
    //m_geomCube.createVBOdata();

    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            instantiateChunkAt(x, z);
            //newChunk->createVBOdata();
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));

    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            if((x + z) % 2 == 0) {
                setGlobalBlockAt(x, 128, z, STONE);
            }
            else {
                setGlobalBlockAt(x, 128, z, DIRT);
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x) {
        setGlobalBlockAt(x, 129, 16, GRASS);
        setGlobalBlockAt(x, 130, 16, GRASS);
        setGlobalBlockAt(x, 129, 48, GRASS);
        setGlobalBlockAt(16, 130, x, GRASS);
    }
    // Add a central column
    for(int y = 129; y < 140; ++y) {
        setGlobalBlockAt(32, y, 32, GRASS);
    }
}




bool Terrain::terrainExpansion(glm::vec3 playerPos){
    // check if player is within 16 blocks of the edge of a chunk

    glm::ivec2 currChunkPos = glm::ivec2(16 * glm::floor(playerPos.x / 16.f), 16 * glm::floor(playerPos.z / 16.f));
    if (!hasChunkAt(playerPos.x, playerPos.z)) {
        Chunk* playerChunk = instantiateChunkAt(currChunkPos.x, currChunkPos.y);
        ProceduralHelp(playerChunk);

    }

    uPtr<Chunk>& playerChunk = getChunkAt(playerPos.x, playerPos.z);
    // glm::ivec2 currChunkPos = glm::ivec2(16 * glm::floor(playerPos.x / 16.f), 16 * glm::floor(playerPos.y / 16.f));
    // int currChunkX = currChunkPos.x;
    // int currChunkZ = currChunkPos.y;
    int playerChunkX = playerChunk->minX;
    int playerChunkZ = playerChunk->minZ;
    int surroundingChunks = 10;
    std::vector<Chunk*> dirtyChunks;
    for (int x = playerChunkX - surroundingChunks * CHUNK_WIDTH; x < playerChunkX + surroundingChunks * CHUNK_WIDTH; x += CHUNK_WIDTH) {
        for (int z = playerChunkZ - surroundingChunks * CHUNK_WIDTH; z < playerChunkZ + surroundingChunks * CHUNK_WIDTH; z += CHUNK_WIDTH) {
            // check up, down, left, right neighbours to see if chunk exists
            for (glm::vec2 neighbour : adjacentBlocks2d) {
                glm::vec2 currBlock = glm::vec2(x + neighbour.x, z + neighbour.y);
                if (!hasChunkAt(currBlock.x, currBlock.y)) {
                    glm::ivec2 currChunkPos = glm::ivec2(16 * glm::floor(currBlock.x / 16.f), 16 * glm::floor(currBlock.y / 16.f));
                    Chunk* currChunk = instantiateChunkAt(currChunkPos.x, currChunkPos.y);
                    ProceduralHelp(currChunk);
                    dirtyChunks.push_back(currChunk);
                    //currChunk->createVBOdata();
                }
            }
        }
    }
    playerChunk->createVBOdata();
    for (auto ptr : dirtyChunks)
    {
        //glm::ivec2 coord = toCoords(key);
        //blockWorkerThreads.push_back(std::thread(&Terrain::blockWorker, this, getChunkAt(coord.x, coord.y).get()));

        ptr->createVBOdata();
    }
}

void Terrain::ProceduralHelp(Chunk* currChunk) {
    int currChunkX = currChunk->minX;
    int currChunkZ = currChunk->minZ;

    std::array<std::pair<int, Biome>, 256> blockInfo;
    int i = 0;

    for (int x = currChunkX; x < currChunkX + CHUNK_WIDTH; ++x) {
        for (int z = currChunkZ; z < currChunkZ + CHUNK_WIDTH; ++z) {
            // generate terrain
            std::pair<int, Biome> temp = generator.genBiome(glm::vec2(x, z));
            int topY = temp.first;
            Biome currBiome = temp.second;
            blockInfo[i++] = temp;

            for (int y = 1; y <= topY; ++y) {
                // cave generation
                if (y >= STONE_BOTTOM && y < STONE_TOP) {
                    BlockType caveBlock = generator.genCave(glm::vec3(x, y, z));
                    setGlobalBlockAt(x, y, z, caveBlock);
                    continue;
                }

                BlockType currBlock = genBlocktype(glm::ivec3(x,y,z), currBiome);
                // grass only grows on top
                if (currBlock == GRASS && y != topY) {
                    currBlock = DIRT;
                }

                // to make ice bergs float for ICELAND biome
                if (currBiome == ICELANDS && topY < STONE_TOP) {
                    for (int i = topY; i <= STONE_TOP; ++i) {
                        setGlobalBlockAt(x, i, z, WATER);
                    }
                } else {
                    setGlobalBlockAt(x, y, z, currBlock);
                }
            }

            // generate water - except ice biome
           if (topY < WATER_TABLE && currBiome != ICELANDS) {
               for (int y = topY; y <= WATER_TABLE; ++y) {
                   setGlobalBlockAt(x, y, z, WATER);
               }
           }

            // bedrock at the very bottom
            setGlobalBlockAt(x, 0, z, BEDROCK);
        }
    }

    // generate features (trees, pyramids, etc)
    i = 0;
    for (int x = currChunkX; x < currChunkX + CHUNK_WIDTH; ++x) {
        for (int z = currChunkZ; z < currChunkZ + CHUNK_WIDTH; ++z) {
            genContFeatures(glm::ivec3(x, blockInfo[i].first, z), blockInfo[i].second);
            if ((x - currChunkX) % 5 == 0 && (z - currChunkZ) % 5 == 0) {
                genSparseFeatures(glm::ivec3(x, blockInfo[i].first, z), blockInfo[i].second);
            }
            i++;
        }
    }
};

BlockType Terrain::genBlocktype(glm::ivec3 pos, Biome biome) {
    int height = pos.y;

    // underground - stones
    if (height >= 0 && height < STONE_TOP) {
        return STONE;
    }

    switch (biome) {
    case MOUNTAINS:
        // too high - snow
        if (height >= generator.snowHeight(pos) && height < 255) {
            return SNOW;
        }
        return STONE;

    case GRASSLANDS:
        return GRASS;

    case DESERT:
        return SAND;

    case ICELANDS:
        // too high - snow
        if (height >= generator.snowHeight(pos) && height < 255) {
            return SNOW;
        }
        return ICE;
    }
}

void Terrain::genSparseFeatures(glm::ivec3 pos, Biome biome) {
    BlockType featureType;
    int featureLength;
    int level = 1;
    glm::vec3 featurePos;
    bool featureAllowed = true;

    switch (biome) {
    case MOUNTAINS:
        // mushroom trees
        for (int x = -2; x <= 2; ++x) {
            for (int z = -2; z <= 2; ++z) {
                if (!hasChunkAt(pos.x + x, pos.z + z)) {
                    featureAllowed = false;
                    break;
                }

                for (int y = 1; y <= 6; ++y) {
                    if (getGlobalBlockAt(pos.x + x, pos.y + y, pos.z + z) != EMPTY) {
                        featureAllowed = false;
                        break;
                    }
                }
            }
        }

        featureLength = generator.genTreeHeight(pos);
        if (pos.y < 150 && featureAllowed && featureLength > 0) {
            // stem
            for (int i = 0; i < featureLength; ++i) {
                setGlobalBlockAt(pos.x, pos.y + i, pos.z, MUSH_STEM);
            }

            // hat
            for (int y = -2; y <= 1; ++y) {
                if (y == 1) {
                    for (int x = -1; x <= 1; ++x) {
                        for (int z = -1; z <= 1; ++z) {
                            setGlobalBlockAt(pos.x + x, pos.y + featureLength + 1, pos.z + z, MUSH_HAT);
                        }
                    }
                }
                else {
                    for (int x = -2; x <= 2; ++x) {
                        for (int z = -2; z <= 2; ++z) {
                            if ((abs(x) == 2 && abs(z) != 2) || (abs(z) == 2 && abs(x) != 2)) {
                                setGlobalBlockAt(pos.x + x, pos.y + featureLength + y, pos.z + z, MUSH_HAT);
                            }
                        }
                    }
                }
            }
        }

        break;
    case GRASSLANDS:
        // tree
        // check for space
        for (int x = -5; x <= 5; ++x) {
            for (int z = -5; z <= 5; ++z) {
                if (!hasChunkAt(pos.x + x, pos.z + z)) {
                    featureAllowed = false;
                    break;
                }

                for (int y = 1; y <= 5; ++y) {
                    if (getGlobalBlockAt(pos.x + x, pos.y + y, pos.z + z) != EMPTY) {
                        featureAllowed = false;
                        break;
                    }
                }
            }
        }

        if (featureAllowed) {
            featureLength = generator.genTreeHeight(pos);
            if (featureLength > 0) {
                // trunk
                for (int i = 1; i <= featureLength; ++i) {
                    setGlobalBlockAt(pos.x, pos.y + i, pos.z, WOOD);
                }

                // leaves
                for (int x = -2; x <= 2; ++x) {
                    for (int z = -2; z <= 2; ++z) {
                        if (x == 0 && z == 0)
                            continue;
                        for (int y = -2; y <= 1; ++y) {
                            if (y <= -1) {
                                if (abs(x) == 2 && abs(z) == 2 && abs(generator.random(glm::vec2(pos.x + x, pos.y + z)).x) > 0.5)
                                    continue;
                                setGlobalBlockAt(pos.x + x, pos.y + featureLength + y, pos.z + z, LEAF);
                            }
                            else {
                                if (abs(x) <= 1 && abs(z) <= 1) {
                                    if (abs(x) == 1 && abs(z) == 1) {
                                        if (y == 1) //always cut out top edge
                                            continue;
                                        else if (abs(generator.random(glm::vec2(pos.x + x, pos.y + z)).x) > 0.5)
                                            continue;
                                    }
                                    setGlobalBlockAt(pos.x + x, pos.y + featureLength + y, pos.z + z, LEAF);
                                }
                            }
                        }
                    }
                }
                setGlobalBlockAt(pos.x, pos.y + featureLength + 1, pos.z, LEAF);
            }
        }

        break;
    case DESERT:
        // pyramid
        featureLength = generator.genPyramid(pos);
        if (featureLength > 0) {
            // check space
            for (int x = -1 * featureLength/2; x < featureLength/2; ++x) {
                for (int z = -1 * featureLength/2; z < featureLength/2; ++z) {
                    if (!hasChunkAt(pos.x + x, pos.z + z)) {
                        featureAllowed = false;
                        break;
                    }

                    if (getGlobalBlockAt(pos.x + x, pos.y + 1, pos.z + z) == SANDSTONE) {
                        featureAllowed = false;
                        break;
                    }
                }
            }

            if (featureAllowed) {
                for (int i = featureLength; i > 0; i -= 2) {
                    for (int x = -1 * i/2; x < i/2; ++x) {
                        for (int z = -1 * i/2; z < i/2; ++z) {
                            setGlobalBlockAt(pos.x + x, pos.y + level, pos.z + z, SANDSTONE);
                        }
                    }
                    ++level;
                }
            }
        }

        // cactus
        featureLength = generator.genTreeHeight(pos);
        if (featureLength > 5) {
            for (int x = -5; x <= 5; ++x) {
                for (int z = -5; z <= 5; ++z) {
                    if (!hasChunkAt(pos.x + x, pos.z + z)) {
                        featureAllowed = false;
                        break;
                    }

                    for (int y = 1; y <= 5; ++y) {
                        if (getGlobalBlockAt(pos.x + x, pos.y + y, pos.z + z) != EMPTY) {
                            featureAllowed = false;
                            break;
                        }
                    }
                }
            }

            if (featureAllowed) {
                for (int i = 0; i < featureLength - 3; ++i) {
                    setGlobalBlockAt(pos.x, pos.y + i, pos.z, CACTUS);
                }
            }
        }

        // bush
        featureType = generator.genFlower(pos);
        if(featureType != EMPTY) {
            setGlobalBlockAt(pos.x, pos.y + 1, pos.z, BUSH);
        }

        break;
    case ICELANDS:
        // igloo - maybe


        break;
    break;
    }
}


void Terrain::genContFeatures(glm::ivec3 pos, Biome biome) {
    BlockType featureType;

    switch(biome) {
    case MOUNTAINS:
        // little mushrooms
        featureType = generator.genMush(pos);
        if (featureType != EMPTY && pos.y < 150) {
            setGlobalBlockAt(pos.x, pos.y + 1, pos.z, featureType);
        }

        break;
    case GRASSLANDS:
        // flower
        featureType = generator.genFlower(pos);
        if (featureType != EMPTY) {
            setGlobalBlockAt(pos.x, pos.y + 1, pos.z, featureType);
        }

        break;
    case DESERT:
        break;
    case ICELANDS:
        break;
    break;
    }
}


void Terrain::multithreadedWork(glm::vec3 playerPos, glm::vec3 playerPosPrev ) {
    // m_tryExpansiontimer += dT;

    // if (m_tryExpansionTimer < 0.5f) {

    // }

    tryExpansion(playerPos, playerPosPrev);
    checkThreadResults();
    // m_tryExpansionTimer = 0.f;
}


std::vector<glm::ivec2> getTerrainGenerationZonesAround(glm::ivec2 pos, int n) {
    std::vector<glm::ivec2> tgzs = {};
    int halfWidth = glm::floor(n / 2.f) * 64;

    for (int i = pos.x - halfWidth; i <= pos.x + halfWidth; i += 64) {
        for (int j = pos.y - halfWidth; j <= pos.y + halfWidth; j+= 64) {
            tgzs.push_back(glm::ivec2(i, j));
        }
    }

    return tgzs;
}


std::vector<glm::ivec2> diffVectors(std::vector<glm::ivec2> a, std::vector<glm::ivec2> b) {
    std::vector<glm::ivec2> diff;

    for (glm::ivec2 vecB : b) {
        bool match = false;
        for (glm::ivec2 vecA : a) {
            if (vecA.x == vecB.x && vecA.y == vecB.y) {
                match = true;
                break;
            }
        }

        if (!match) {
            diff.push_back(vecB);
        }
    }

    return diff;
}

bool Terrain::hasTerrainGenerationZoneAt(glm::ivec2 zone) {
    return m_generatedTerrain.find(toKey(zone.x, zone.y)) != m_generatedTerrain.end();
}

void Terrain::tryExpansion(glm::vec3 playerPos, glm::vec3 playerPosPrev) {
    glm::ivec2 currZone = glm::ivec2(64.f * glm::floor(playerPos.x / 64.f), 64.f * glm::floor(playerPos.y / 64.f));
    glm::ivec2 prevZone = glm::ivec2(64.f * glm::floor(playerPosPrev.x / 64.f), 64.f * glm::floor(playerPosPrev.y / 64.f));

    std::vector<glm::ivec2> currTGZs = getTerrainGenerationZonesAround(currZone, 10);
    std::vector<glm::ivec2> prevTGZs = getTerrainGenerationZonesAround(prevZone, 10);

    std::vector<glm::ivec2> newZones = firstTick ? currTGZs : diffVectors(prevTGZs, currTGZs);
    std::vector<glm::ivec2> oldZones = diffVectors(currTGZs, prevTGZs);

    for (glm::ivec2 newZone : newZones) {
        if (!hasTerrainGenerationZoneAt(newZone)) {
            m_generatedTerrain.insert(toKey(newZone.x, newZone.y));

            for (int x = 0; x < 64; x += 16) {
                for (int z = 0; z < 64; z += 16) {
                    newChunks.insert(toKey(newZone.x + x, newZone.y + z));
                    // newChunks[toKey(newZone.x + x, newZone.y + z)] = instantiateChunkAt(newZone.x + x, newZone.y + z);
                }
            }
        }
    }


    for (auto & key : newChunks) {
        glm::ivec2 coord = toCoords(key);
        instantiateChunkAt(coord.x, coord.y);
    }

    for (auto & key : newChunks) {
        glm::ivec2 coord = toCoords(key);
        blockWorkerThreads.push_back(std::thread(&Terrain::blockWorker, this, getChunkAt(coord.x, coord.y).get()));
    }

    newChunks.clear();
    firstTick = false;
}



void Terrain::blockWorker(Chunk *chunk) {

    ProceduralHelp(chunk);

    glm::ivec2 chunkWorldPos= glm::ivec2(16 * glm::floor(chunk->minX / 16.f), 16 * glm::floor(chunk->minZ / 16.f));



    chunksWithBlockDataMutex.lock();
    chunksWithBlockData[toKey(chunkWorldPos.x, chunkWorldPos.y)] = chunk;
    //chunksWithBlockDataGenerated[toKey(chunkWorldPos.x, chunkWorldPos.y)] = true; // notes that the block data has been generated for deferencing
    chunksWithBlockDataMutex.unlock();
}


void Terrain::checkThreadResults() {
    // Send Chunks that have been given VBO data
    // by VBOWorkers for VBO data
    // m_chunksThatHaveBlockDataLock.lock();
    // spawnVBOWorkers(m_chunksThatHaveBlockData);
    // m_chunksThatHaveBlockData.clear();
    // m_chunksThatHaveBlockDataLock.unlock();

    // // Collect the Chunks that have been given VBO data
    // // by VBOWorkers and send that VBO data to the GPU
    // m_chunksThatHaveVBOsLock.lock();
    // for (ChunkVBOData &cd : m_chunksThatHaveVBOs) {
    //     cd.mp_chunk->create(cd.m_vboDataOpaque, cd.m_idxDataOpaque,
    //                         cd.m_vboDataTransparent, cd.m_idxDataTransparent);
    // }
    // m_chunksThatHaveVBOs.clear();
    // m_chunksThatHaveVBOsLock.unlock();


    chunksWithBlockDataMutex.lock();
    for (auto & [ key, chunk ] : chunksWithBlockData) {
        vboWorkerThreads.push_back(std::thread(&Terrain::VBOWorker, this, chunk));
    }
    chunksWithBlockData.clear();
    chunksWithBlockDataMutex.unlock();

    chunksWithVBODataMutex.lock();
    for (auto & [ key, chunk ] : chunksWithVBOData) {
        chunk->bufferVBOtoGPU();
        // todo: replace ^ with a function that ONLY buffers data to GPU
        // chunk->create();
        // m_chunks[key] = move(chunk);
    }


    chunksWithVBOData.clear();
    chunksWithVBODataMutex.unlock();
}


#if 0
void Terrain::checkThreadResults() {
    // Send Chunks that have been given VBO data
    // by VBOWorkers for VBO data


    // // Collect the Chunks that have been given VBO data
    // // by VBOWorkers and send that VBO data to the GPU


    chunksWithBlockDataMutex.lock();
//    for (auto & [ key, chunk ] : chunksWithVBOData) {
//            chunk->bufferVBOtoGPU();
//            // todo: replace ^ with a function that ONLY buffers data to GPU
//            // chunk->create();
//            // m_chunks[key] = move(chunk);
//    }


    for (auto & [ key, chunk ] : chunksWithBlockData) {

        // check if neighbours also have block data
        bool neighboursGenerated = true;

        for (auto neighbourPair : chunk->m_neighbors) {
            Chunk *neighbour = neighbourPair.second;
            if (!neighbour) {
                // neighbour hasn't been instantiated
                neighboursGenerated = false;
                continue;
            }
            auto it = chunksWithBlockData.find(toKey(neighbour->minX, neighbour->minZ));
            if (it == chunksWithBlockData.end()) {
                // key doesn't exist
                neighboursGenerated = false;
            }
        }
        if (neighboursGenerated) {
            vboWorkerThreads.push_back(std::thread(&Terrain::VBOWorker, this, chunk));
            //chunksWithVBOGenerated[toKey(chunk->minX, chunk->minZ)] = true;
            //chunksWithBlockDataGenerated[toKey(chunk->minX, chunk->minZ)] = true;
            // check if neighbours VBOs have been generated, if so can free memory of current chunk
            bool neighboursVBOGenerated = true;
            for (auto neighbourPair : chunk->m_neighbors) {
                Chunk *neighbour = neighbourPair.second;
                if (!neighbour) {
                    // neighbour hasn't been instantiated
                    neighboursVBOGenerated = false;
                    continue;
                }
                auto it = chunksWithVBOGenerated.find(toKey(neighbour->minX, neighbour->minZ));
                if (it == chunksWithVBOGenerated.end()) {
                    // key doesn't exist
                    neighboursVBOGenerated = false;
                }
            }
            if (neighboursVBOGenerated) {
                chunksWithBlockData.erase(key);
            }
            //chunksWithBlockData.erase(key);
        }

    }

    chunksWithBlockData.clear();
    chunksWithBlockDataMutex.unlock();

    chunksWithVBODataMutex.lock();
    for (auto & [ key, chunk ] : chunksWithVBOData) {
        chunk->bufferVBOtoGPU();
        chunksWithVBOGenerated[toKey(chunk->minX, chunk->minZ)] = true;
        // todo: replace ^ with a function that ONLY buffers data to GPU
        // chunk->create();
        // m_chunks[key] = move(chunk);
    }


    chunksWithVBOData.clear();
    chunksWithVBODataMutex.unlock();
}
#endif


// bool Terrain::areNeighboursInMap(Chunk* chunk, std::unordered_map) {

// }
void Terrain::VBOWorker(Chunk *chunk) {
    chunk->createVBOdata();

    glm::ivec2 chunkWorldPos = glm::ivec2(16 * glm::floor(chunk->minX / 16.f), 16 * glm::floor(chunk->minZ / 16.f));

    chunksWithVBODataMutex.lock();
    chunksWithVBOData[toKey(chunkWorldPos.x, chunkWorldPos.y)] = chunk;
    chunksWithVBODataMutex.unlock();
}


// VVVVVVV Put in main thread
void Chunk::bufferVBOtoGPU() {
    bufferDataTrns(opqData, opqIdx);
    bufferDataOpq(trnsData, trnsIdx);
}




