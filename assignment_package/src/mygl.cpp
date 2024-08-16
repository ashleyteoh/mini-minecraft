#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>

#define MIN_BLOCK_HIT 3.0f;


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this), m_progInstanced(this), m_progOverlay(this),
      m_terrain(this), m_player(glm::vec3(48.f, 129.f, 48.f), m_terrain), m_frame(this, width(), height(), devicePixelRatio()), m_quad(this), raycaster(), m_textureMap(this), m_globalTime(0),
        numGrass(10), numDirt(10), numStone(10),
        numSand(10), numWood(10), numLeaf(10), numSnow(10),
        numMushHat(10), numMushStem(10),
        numSandStone(10), numCactus(10), numIce(10),
        currBlockType(GRASS), openInventory(false)

{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible

    m_lastFrameTime = QDateTime::currentMSecsSinceEpoch();
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");
    m_progOverlay.create(":/glsl/fluidoverlay.vert.glsl", ":/glsl/fluidoverlay.frag.glsl");

    m_quad.createVBOdata();

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    m_frame.create();

    m_textureMap = Texture(this);
    // TODO

    QString path = QDir::currentPath();
    path = path.left(path.lastIndexOf("/"));
    QString texturePath = path + "/textures/minecraft_textures_all.png";
    qDebug() << texturePath;

    m_textureMap.create(texturePath.toStdString().c_str());
    m_textureMap.loadTexture(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)
    m_progLambert.setUnifMat4("u_ViewProj", viewproj);
    m_progFlat.setUnifMat4("u_ViewProj", viewproj);
    m_progInstanced.setUnifMat4("u_ViewProj", viewproj);
    m_progOverlay.setUnifMat4("u_ViewProj", viewproj);

    printGLErrorLog();

    // update the m_frame
    m_frame.resize(width(), height(), devicePixelRatio());
    m_frame.destroy();
    m_frame.create();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    glm::vec3 prevPlayerPos = m_player.mcr_position;
//    m_terrain.terrainExpansion(m_player.m_position);

    //m_globalTime += m_timer.interval();
    // m_terrain.terrainExpansion(m_player.m_position);

    qint64 currentFrameTime = QDateTime::currentMSecsSinceEpoch();
    qint64 dT = currentFrameTime - m_lastFrameTime;
    m_lastFrameTime = currentFrameTime;
    float dTSeconds = dT / 1000.0f;

    m_player.tick(dTSeconds, m_inputs); // Call tick with delta-time in seconds


    m_terrain.multithreadedWork(m_player.mcr_position, prevPlayerPos);

    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data
    sendInventoryDataToGUI(); // Update inventory quanities
}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

void MyGL::sendInventoryDataToGUI() const {
    emit sig_sendNumGrass(numGrass);
    emit sig_sendNumDirt(numDirt);
    emit sig_sendNumStone(numStone);
    emit sig_sendNumSand(numSand);
    emit sig_sendNumWood(numWood);
    emit sig_sendNumLeaf(numLeaf);
    emit sig_sendNumSnow(numSnow);
    emit sig_sendNumMushStem(numMushStem);
    emit sig_sendNumMushHat(numMushHat);
    emit sig_sendNumSandstone(numSandStone);
    emit sig_sendNumCactus(numCactus);
    emit sig_sendNumIce(numIce);
}

void MyGL::renderPostProcess() {
    m_frame.bindFrameBuffer();
    m_frame.bindToTextureSlot(0);

    // check if the player is under some type of fluid
    if (m_terrain.hasChunkAt(m_player.m_position.x, m_player.m_position.y)) {
        if (m_terrain.getGlobalBlockAt(m_player.m_position) == WATER) {
            m_progOverlay.useMe();
            // set mode = 1 in the overlay shader
            m_progOverlay.setUnifInt("fluid", 2);
            m_progOverlay.drawTranslucent(m_quad);
        }
        else if(m_terrain.getGlobalBlockAt(m_player.m_position) == LAVA) {
            m_progOverlay.useMe();
            // mode = 2
            m_progOverlay.setUnifInt("fluid", 2);
            m_progOverlay.drawTranslucent(m_quad);
        }
    }
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();
    m_progLambert.setUnifMat4("u_ViewProj", viewproj);
    m_progFlat.setUnifMat4("u_ViewProj", viewproj);
    m_progInstanced.setUnifMat4("u_ViewProj", viewproj);
    m_progLambert.setUnifMat4("u_ModelInvTr", glm::mat4());
    m_progLambert.setUnifMat4("u_Model", glm::mat4());

    m_progLambert.setUnifInt("u_Time", ++m_globalTime);
    m_progLambert.setUnifVec3("u_PlayerPos", m_player.mcr_position);

    m_progOverlay.setUnifMat4("u_Model", glm::mat4());
    m_progOverlay.setUnifMat4("u_ViewProj", viewproj);

    renderTerrain();

    renderPostProcess();

    // world axis - comment out when video
    glDisable(GL_DEPTH_TEST);
    m_progFlat.setUnifMat4("u_Model", glm::mat4());

   // m_progFlat.draw(m_worldAxes);
    m_textureMap.bind(0);

    glEnable(GL_DEPTH_TEST);
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain() {
    m_terrain.draw(glm::vec2(m_player.m_position.x, m_player.m_position.z), &m_progLambert);
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }

    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_inputs.rightPressed = true;
    } else if (e->key() == Qt::Key_Left) {
        m_inputs.leftPressed = true;
    } else if (e->key() == Qt::Key_Up) {
        m_inputs.upPressed = true;
    } else if (e->key() == Qt::Key_Down) {
        m_inputs.downPressed = true;

    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;

    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;

    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;

    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;

    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;

    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;

    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    }
    if (e->key() == Qt::Key_F) {
        m_player.flightMode = !m_player.flightMode;
    }
    if (e->key() == Qt::Key_I) {
        openInventory = !openInventory;
        if (openInventory) {
            std::cout << "show inventory" << std::endl;
        } else {
            std::cout << "hide inventory" << std::endl;
        }
        emit sig_inventoryOpenClose(openInventory);
    }

}

void MyGL::keyReleaseEvent(QKeyEvent *e) {

    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;

    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;

    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;

    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    }
    else if (e->key() == Qt::Key_Right) {
        m_inputs.rightPressed = false;
    } else if (e->key() == Qt::Key_Left) {
        m_inputs.leftPressed = false;
    } else if (e->key() == Qt::Key_Up) {
        m_inputs.upPressed = false;
    } else if (e->key() == Qt::Key_Down) {
        m_inputs.downPressed = false;
    }

}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // Currently a bit laggy

    float dx = (this->width() * 0.5 - e->pos().x()) / width();
    float dy = (this->height() * 0.5 - e->pos().y()) / height();
    m_player.rotateOnUpGlobal(dx * 360 * 0.005f);
    m_player.rotateOnRightLocal(dy * 360 * 0.005f);
    moveMouseToCenter();

}


bool MyGL::checkBlockInventory(BlockType currBlockType) {
    if (currBlockType == GRASS && numGrass > 0) {
        return true;
    } else if (currBlockType == DIRT && numDirt > 0) {
        return true;
    } else if (currBlockType == STONE && numStone > 0) {
        return true;
    } else if (currBlockType == SNOW && numSnow > 0) {
        return true;
    } else if (currBlockType == WOOD && numWood > 0) {
       return true;
    }  else if (currBlockType == LEAF && numLeaf > 0) {
        return true;
     } else if (currBlockType == SAND && numSand > 0) {
        return true;
     } else if (currBlockType == MUSH_HAT && numMushHat > 0) {
        return true;
     } else if (currBlockType == MUSH_STEM && numMushStem > 0) {
        return true;
     } else if (currBlockType == SANDSTONE && numSandStone > 0) {
        return true;
     } else if (currBlockType == CACTUS && numCactus > 0) {
        return true;
     } else if (currBlockType == ICE && numIce > 0) {
        return true;
     }


    return false;
}

void MyGL::decreaseInventory() {
    if (currBlockType == GRASS) {
        numGrass--;
    } else if (currBlockType == DIRT) {
        numDirt--;
    } else if (currBlockType == STONE) {
        numStone--;
    } else if (currBlockType == SNOW) {
        numSnow--;
    } else if (currBlockType == WOOD) {
        numWood--;
    } else if (currBlockType == LEAF) {
        numLeaf--;
    } else if (currBlockType == SAND) {
        numSand--;
    } else if (currBlockType == ICE) {
        numIce--;
    } else if (currBlockType == MUSH_HAT) {
        numMushHat--;
    } else if (currBlockType == MUSH_STEM) {
        numMushStem--;
    } else if (currBlockType == CACTUS) {
        numCactus--;
    } else if (currBlockType == SANDSTONE) {
        numSandStone--;
    }
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // TODO

    float outDist = 0.f;
    glm::ivec3 outBlockHit = glm::ivec3();
    glm::vec3 rayDirection = glm::normalize(m_player.mcr_camera.m_forward) * 3.0f;
    float ifAxis = -1;

    if (raycaster.gridMarch(m_player.mcr_camera.mcr_position, rayDirection, m_terrain, &outDist, &outBlockHit, &ifAxis)) {

        if (e->button() == Qt::LeftButton) {
            std::cout << ifAxis << std::endl;

            BlockType blockType = m_terrain.getGlobalBlockAt(outBlockHit.x, outBlockHit.y, outBlockHit.z);

            if(blockType != BEDROCK) { // break the block unless it is bedrock
                m_terrain.setGlobalBlockAt(outBlockHit.x, outBlockHit.y, outBlockHit.z, EMPTY);

                m_terrain.getChunkAt(outBlockHit.x, outBlockHit.z).get()->createVBOdata();
                m_terrain.getChunkAt(outBlockHit.x, outBlockHit.z).get()->bufferVBOtoGPU();

                if (blockType == GRASS) {
                    numGrass++;
                } else if (blockType == DIRT) {
                    numDirt++;
                } else if (blockType == STONE) {
                    numStone++;
                 } else if (blockType == SAND) {
                     numSand++;
                 } else if (blockType == WOOD) {
                     numWood++;
                 } else if (blockType == LEAF) {
                     numLeaf++;
                } else if (blockType == SNOW) {
                    numSnow++;
                } else if (blockType == ICE) {
                    numIce++;
                } else if (blockType == MUSH_HAT) {
                    numMushHat++;
                } else if (blockType == MUSH_STEM) {
                    numMushStem++;
                } else if (blockType == CACTUS) {
                    numCactus++;
                } else if (blockType == SANDSTONE) {
                    numSandStone++;
                }

                // std::cout << "remove block" << std::endl;
            }
        }
        else if (e->button() == Qt::RightButton) {
        // else {

            if (checkBlockInventory(currBlockType)) {

                std::cout << ifAxis << std::endl;
                if (ifAxis == 2) {
                    if (m_terrain.getGlobalBlockAt(outBlockHit.x, outBlockHit.y, outBlockHit.z - glm::sign(rayDirection.z)) == EMPTY) {
                        m_terrain.setGlobalBlockAt(outBlockHit.x, outBlockHit.y, outBlockHit.z - glm::sign(rayDirection.z), currBlockType);

                        m_terrain.getChunkAt(outBlockHit.x, outBlockHit.z).get()->createVBOdata();
                        m_terrain.getChunkAt(outBlockHit.x, outBlockHit.z).get()->bufferVBOtoGPU();

                        decreaseInventory();

                        std::cout << "create" << std::endl;

                    }
                } else if (ifAxis == 1) {
                    if (m_terrain.getGlobalBlockAt(outBlockHit.x, outBlockHit.y - glm::sign(rayDirection.y), outBlockHit.z) == EMPTY) {
                        m_terrain.setGlobalBlockAt(outBlockHit.x, outBlockHit.y - glm::sign(rayDirection.y), outBlockHit.z, currBlockType);

                        m_terrain.getChunkAt(outBlockHit.x, outBlockHit.z).get()->createVBOdata();
                        m_terrain.getChunkAt(outBlockHit.x, outBlockHit.z).get()->bufferVBOtoGPU();

                        decreaseInventory();

                        std::cout << "create" << std::endl;

                    }
                } else if (ifAxis == 0) {
                    if (m_terrain.getGlobalBlockAt(outBlockHit.x - glm::sign(rayDirection.x), outBlockHit.y, outBlockHit.z) == EMPTY) {
                        m_terrain.setGlobalBlockAt(outBlockHit.x - glm::sign(rayDirection.x), outBlockHit.y, outBlockHit.z, currBlockType);

                        m_terrain.getChunkAt(outBlockHit.x, outBlockHit.z).get()->createVBOdata();
                        m_terrain.getChunkAt(outBlockHit.x, outBlockHit.z).get()->bufferVBOtoGPU();

                        decreaseInventory();

                        std::cout << "create" << std::endl;

                    }
                }

                // m_terrain.setGlobalBlockAt(outBlockHit.x, outBlockHit.y, outBlockHit.z, GRASS);
                // m_inputs.rightMousePressed = true;
            }
        }

    }
}


