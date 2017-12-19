/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Model struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <string>
#include "material.h"
#include "mesh.h"
#include <glm/mat4x4.hpp>
#include <memory>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A model
 * -------------------------------------------------------------------------- */
struct Model
{
    Model();
    std::string name;
    std::shared_ptr<Material> material;
    std::shared_ptr<Mesh> mesh;
    glm::mat4 worldTransform;
};

} // namespace kuu
