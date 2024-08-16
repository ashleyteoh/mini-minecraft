#include "player.h"
#include <iostream>
#include <QString>
#include <limits>

// TODO - change back walk speed
#define WALK_SPEED 15.f
#define JUMP_SPEED 10.f
#define GRAVITY -10.f
#define SENSITIVITY 3.0f


Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos), m_velocity(0,0,0), m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)), mcr_terrain(terrain),
    mcr_camera(m_camera), raycaster()
{}

Player::~Player()
{}

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain, input);
}

void Player::processInputs(InputBundle &inputs) {
    // TODO: Update the Player's velocity and acceleration based on the
    // state of the inputs.

    if (flightMode) {
        if(inputs.wPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration += m_forward * WALK_SPEED;
        } if (!inputs.wPressed) {
            m_acceleration =  glm::vec3(0);
        }
        if(inputs.sPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration -= m_forward * WALK_SPEED;
        }
        if(inputs.dPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration += m_right * WALK_SPEED;
        }
        if(inputs.aPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration -= m_right * WALK_SPEED;
        }
        if (inputs.ePressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration += m_up * WALK_SPEED;
        }
        if (inputs.qPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration -= m_up * WALK_SPEED;
        }
    }
    else {
        if(inputs.wPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration += glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z)) * WALK_SPEED;
        } if (!inputs.wPressed) {
            m_acceleration =  glm::vec3(0);
        }
        if(inputs.sPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration -= glm::normalize(glm::vec3(m_forward.x, 0, m_forward.z)) * WALK_SPEED;
        }
        if(inputs.dPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration += glm::normalize(glm::vec3(m_right.x, 0, m_right.z)) * WALK_SPEED;
        }
        if(inputs.aPressed) {
            m_acceleration = glm::vec3(0);
            m_acceleration -= glm::normalize(glm::vec3(m_right.x, 0, m_right.z)) * WALK_SPEED;
        }
        if(inputs.spacePressed && this->isGrounded()) {
            m_velocity.y = JUMP_SPEED;
        }

    }

    // Look around rotations
    if (inputs.rightPressed) {
        this->rotateOnUpGlobal(-SENSITIVITY);
    }
    if (inputs.leftPressed) {
        this->rotateOnUpGlobal(SENSITIVITY);
    }
    if (inputs.upPressed) {
        this->rotateOnRightLocal(SENSITIVITY);
    }
    if (inputs.downPressed) {
        this->rotateOnRightLocal(-SENSITIVITY);
    }

}


void Player::computePhysics(float dT, const Terrain &terrain, InputBundle &input) {
    // TODO: Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.

    m_velocity *= 0.95f; // friction + drag
    m_velocity += m_acceleration * dT;
    glm::vec3 rayDirection = m_velocity * dT;

    // handle collision & gravity while the player is in non-flightmode
    if (!flightMode) {
        if (!isGrounded())
            m_velocity.y += GRAVITY * dT;
        else
            m_velocity.y = glm::max(0.f, m_velocity.y);
        m_velocity += m_acceleration * dT;

        rayDirection = m_velocity * dT;
        detectCollision(&rayDirection, terrain);


        m_velocity = rayDirection / glm::max(0.00001f, dT); // idk
    }
    this->moveAlongVector(rayDirection);


}

void Player::detectCollision(glm::vec3 *rayDirection, const Terrain &terrain) {

    glm::ivec3 outBlockHit = glm::ivec3();
    float outDist = 0.f;

    for (const glm::vec3 &corner : playerCorners) {
        glm::vec3 worldCorner = corner + this->m_position;
        float ifAxis = -1;
        if (raycaster.gridMarch(worldCorner, *rayDirection, terrain, &outDist, &outBlockHit, &ifAxis)) {
            float distance = glm::min(outDist - 0.005f, abs(glm::length(this->m_position - glm::vec3(outBlockHit))));
            *rayDirection = distance * glm::normalize(*rayDirection);

            // if (ifAxis != -1) {
            //     // Set the component of rayDirection along the collision axis to zero
            //     (*rayDirection)[ifAxis] = 0.f;
            // }
        }
    }
}

bool Player::isGrounded() {

    bool grounded = false;
    for (glm::vec3 p : feet) {
        glm::vec3 footCorner = p + this->m_position;
        footCorner.y -= 0.005f;
        if (this->mcr_terrain.getGlobalBlockAt(footCorner) != EMPTY) {
            grounded = true;
        }

    }
    return grounded;
}





void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
