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

#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

namespace kuu
{

struct Camera;

/**
    Perspective camera frustum.
 **/
class Frustum
{
public:
    /**
       Creates the camera frustum.
       \param camera Camera whose frustum this class creates.
     **/
    Frustum(const Camera& camera,
            const glm::vec4& viewport);

    /**
        Returns the frustum corners.
        \return The corners of the frustum.
     **/
    std::vector<glm::vec3> corners() const;

    /**
        Returns the frustum center point.
        \return The center point of the frustum
     **/
    glm::vec3 centroid() const;

    /**
        Returns the frustum far plane center point.
        \return The far plane center point of frustum.
     **/
    glm::vec3 farCenter() const;

    /**
        Returns the frustum near plane center point.
        \return The near plane center point of frustum.
     **/
    glm::vec3 nearCenter() const;

    /**
        Returns the orthographic shadow matrix that is used to render
        scene from the directional light's view point.
       \param lightDirection The direction of light.
       \param nearClipOffset ????
       \return The shadow camera matrix.
     **/
    glm::mat4 orthoShadowMatrix(
        const glm::vec3 &lightDirection,
        float nearClipOffset) const;

    glm::mat4 perspectiveShadowMatrix(
        const glm::vec3& lightPosition,
        const glm::vec3& lightDirection,
        const float distance,
        const float angle,
        float nearClipOffset) const;

    glm::mat4 pointLightShadowMatrix(const glm::vec3& lightPosition,
        const float distance, const float nearClipOffset) const;

private:
    std::vector<glm::vec3> corners_;
};

} // namespace kuu
