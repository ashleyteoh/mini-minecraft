#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"
#include "../raycaster.h"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(glm::vec3 o, glm::vec3 d)
        : origin(o), direction(d)
    {}
};

class Player : public Entity {
private:
    glm::vec3  m_acceleration;
    Camera m_camera;
    const Terrain &mcr_terrain;
    Raycaster raycaster;


    void computePhysics(float dT, const Terrain &terrain, InputBundle &input);

    bool isGrounded();

    void detectCollision(glm::vec3 *rayDirection, const Terrain &terrain);
    BlockType removeBlock(Terrain *terrain);


public:
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;
    glm::vec3 m_velocity;


    Player(glm::vec3 pos, const Terrain &terrain);
    virtual ~Player() override;

    void processInputs(InputBundle &inputs);
    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;


    bool flightMode = true;

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;



    const std::array<glm::vec3, 12> playerCorners = {
//        glm::vec3(-0.5, 0, -0.5),
//        glm::vec3(-0.5, 0, +0.5),
//        glm::vec3(-0.5, +1, -0.5),
//        glm::vec3(-0.5, +1, +0.5),
//        glm::vec3(-0.5, +1.9, -0.5),
//        glm::vec3(-0.5, +1.9, +0.5),
//        glm::vec3(+0.5, 0, -0.5),
//        glm::vec3(+0.5, 0, +0.5),
//        glm::vec3(+0.5, +1, -0.5),
//        glm::vec3(+0.5, +1, +0.5),
//        glm::vec3(+0.5, +1.9, -0.5),
//        glm::vec3(+0.5, +1.9, +0.5)
        glm::vec3(-0.4, 0, -0.4),
        glm::vec3(-0.4, 0, +0.4),
        glm::vec3(-0.4, +0.9, -0.4),
        glm::vec3(-0.4, +0.9, +0.4),
        glm::vec3(-0.4, +1.9, -0.4),
        glm::vec3(-0.4, +1.9, +0.4),
        glm::vec3(+0.4, 0, -0.4),
        glm::vec3(+0.4, 0, +0.4),
        glm::vec3(+0.4, +0.9, -0.4),
        glm::vec3(+0.4, +0.9, +0.4),
        glm::vec3(+0.4, +1.9, -0.4),
        glm::vec3(+0.4, +1.9, +0.4)
    };

    const std::array<glm::vec3, 4> feet = {
        glm::vec3(- 0.4, 0, - 0.4),
        glm::vec3(- 0.4, 0, + 0.4),
        glm::vec3(+ 0.4, 0, - 0.4),
        glm::vec3(+ 0.4, 0, + 0.4),
    };
};
