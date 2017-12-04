/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Material struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <glm/vec3.hpp>
#include <string>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A material
 * -------------------------------------------------------------------------- */
struct Material
{
    glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
    std::string diffuseMap;
};

} // namespace kuu
