#pragma once

#include <glm_includes.h>
#include "scene/terrain.h"

class Raycaster
{
public:
    Raycaster();
    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit, float *ifAxis);
};


