/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Camera struct.
 * -------------------------------------------------------------------------- */

#include "camera.h"
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace kuu
{

glm::mat4 Camera::worldTransform() const
{
    const glm::quat q = yaw * pitch * roll;
    return glm::translate(glm::mat4(1.0f), glm::vec3(pos)) * glm::mat4_cast(q);
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::inverse(worldTransform());
}

glm::mat4 Camera::projectionMatrix() const
{
    glm::mat4 m = glm::perspective(
        glm::radians(fieldOfview),
        aspectRatio,
        nearPlane,
        farPlane);
    m[1][1] *= -1;
    return m;
}

} // namespace kuu
