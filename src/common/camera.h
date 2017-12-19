/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Camera struct.
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A perspective camera to transform objects from world space into camera
   space and from camera space into NDC space.

   Contains two transforms:
        1) transform camera from local space into world space
        2) transform objects from camera space into NDC space using
           perspective divide.

   Note that projection matrix maps depth from 0 to 1 (Vulkan mapping) and not
   -1 to 1 (OpenGL mapping).

 * -------------------------------------------------------------------------- */
struct Camera
{
    // Updates the camera
    void update();

    // Returns rotation.
    glm::quat rotation() const;
    // Returns the world transform.
    glm::mat4 worldTransform() const;
    // Returns view transform.
    glm::mat4 viewMatrix() const;
    // Returns projection transform.
    glm::mat4 projectionMatrix() const;
    // Returns the camera matrix (projection x view)
    glm::mat4 cameraMatrix() const;

    // Perspective projection parameters
    float fieldOfview = 56.0f;
    float aspectRatio = 1.0f;
    float nearPlane   = 0.1f;
    float farPlane    = 150.0f;

    // World transform parameters
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::quat pitch;
    glm::quat yaw;
    glm::quat roll;

    glm::vec3 tPos = glm::vec3(0.0f, 0.0f, 10.0f);
    glm::quat tPitch;
    glm::quat tYaw;
    glm::quat tRoll;

    glm::vec3 move;
};

} // namespace kuu
