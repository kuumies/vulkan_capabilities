/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Light struct.
 * -------------------------------------------------------------------------- */

#include "light.h"
#include "frustum.h"

namespace kuu
{

/* -------------------------------------------------------------------------- */

glm::mat4 Light::orthoShadowMatrix(const Camera& camera,
                                   const glm::vec4& viewport,
                                   float nearClipOffset) const
{
    Frustum frustum(camera, viewport);
    return frustum.orthoShadowMatrix(-dir, nearClipOffset);
}

} // namespace kuu
