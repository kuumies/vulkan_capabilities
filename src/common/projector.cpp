/**
    Copyright (c) 2016 Kuu

    Licensed to the Apache Software Foundation (ASF) under one
    or more contributor license agreements.  See the NOTICE file
    distributed with this work for additional information
    regarding copyright ownership.  The ASF licenses this file
    to you under the Apache License, Version 2.0 (the
    "License"); you may not use this file except in compliance
    with the License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing,
    software distributed under the License is distributed on an
    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
    KIND, either express or implied.  See the License for the
    specific language governing permissions and limitations
    under the License.
 **/

#include "projector.h"
#include <iostream>

namespace kuu
{

Projector::Projector(const Camera& camera, const glm::vec4 &viewport)
    : camera_(camera)
    , viewport_(viewport)
{}

glm::vec2 Projector::project(const glm::vec3& p) const
{
    const glm::mat4 camMat = camera_.cameraMatrix();
    if (camMat == glm::mat4(1.0f))
    {
        std::cerr << "Camera matrix is an identity matrix.";
        return glm::vec2();
    }

    glm::vec4 v = camMat * glm::vec4(p.x, p.y, p.z, 1.0f);

    if (v.w == 0.0f)
    {
        std::cerr << "Failed to transform point to clip space." << std::endl;
        return glm::vec2();
    }

    glm::vec3 window(v);
    window /= v.w;

    if (v.z < 0.0f)
    {
        std::cerr << "Persepective divide failed." << std::endl;
        return glm::vec2();
    }

    const glm::vec4 vp = viewport_;
    float x = vp.x + (vp.z * (window.x + 1.0f)) / 2.0f;
    float y = vp.y + (vp.w * (window.y + 1.0f)) / 2.0f;

    return glm::vec2(x, vp.w - y);
}

glm::vec2 Projector::project(float x, float y, float z) const
{
    return project(glm::vec3(x, y, z));
}

glm::vec3 Projector::unproject(const glm::vec3& point, bool topDown) const
{
    const glm::vec4 vp = viewport_;

    glm::vec3 originalPoint(point.x, point.y, point.z);

    // Flip y-axis if the point is down-top
    if (!topDown)
        originalPoint = glm::vec3(originalPoint.x,
                                  vp.w - originalPoint.y,
                                  originalPoint.z);

    const glm::mat4 camMat = camera_.cameraMatrix();
    if (camMat == glm::mat4(1.0f))
    {
        std::cerr << "Camera matrix is an identity matrix.";
        return glm::vec3();
    }
    glm::mat4 invCameraMatrix = glm::inverse(camMat);
    if (invCameraMatrix == glm::mat4(1.0f))
    {
        std::cerr << "Failed to find inverse of camera matrix." << std::endl;
        return glm::vec3();
    }

    // Map x and y from window coordinates
    float a = (originalPoint.x - vp.x) / vp.z;
    float b = (originalPoint.y - vp.y) / vp.w;
    float c = originalPoint.z;

    // Map to range -1 to 1
    a = a * 2.0f - 1.0f;
    b = b * 2.0f - 1.0f;
    c = c * 2.0f - 1.0f;

    glm::vec4 p = invCameraMatrix * glm::vec4(a, b, c, 1.0f);

    if (p.w == 0.0f)
    {
        std::cerr << "Inverse projection failed." << std::endl;
        return glm::vec3();
    }
    return glm::vec3(p.x, p.y, p.z) / p.w;
}

glm::vec3 Projector::unproject(float x, float y, float z, bool topDown) const
{
    return unproject(glm::vec3(x, y, z), topDown);
}

Ray Projector::viewportRay(const glm::vec2& pos) const
{
    glm::vec3 nearPlane(pos.x, pos.y, 0.0);
    nearPlane = unproject(nearPlane, false);

    glm::vec3 farPlane(pos.x, pos.y, 1.0);
    farPlane = unproject(farPlane, false);

    return Ray(nearPlane, farPlane - nearPlane);
}

} // namespace kuu
