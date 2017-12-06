/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Light struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <glm/vec4.hpp>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A light.
 * -------------------------------------------------------------------------- */
struct Light
{
    glm::vec4 dir       = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    glm::vec4 intensity = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

} // namespace kuu
