/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Model struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <string>
#include "material.h"
#include "mesh.h"
#include <glm/mat4x4.hpp>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A model
 * -------------------------------------------------------------------------- */
struct Model
{
    std::string name;
    glm::mat4 worldTransform;
    Material material;
    Mesh mesh;
};

} // namespace kuu
