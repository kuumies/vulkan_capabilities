/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Light struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace kuu
{

/* -------------------------------------------------------------------------- */

struct Camera;

/* -------------------------------------------------------------------------- *
   A light.
 * -------------------------------------------------------------------------- */
struct Light
{
    glm::mat4 orthoShadowMatrix(const Camera& camera,
                                const glm::vec4& viewport,
                                float nearClipOffset) const;

    glm::vec4 dir       = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    glm::vec4 intensity = glm::vec4(2, 2, 2, 1.0f);
};

} // namespace kuu
