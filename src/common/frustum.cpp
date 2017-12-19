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

#include "frustum.h"

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>

#include "bounding_box.h"
#include "camera.h"
#include "projector.h"

namespace kuu
{

Frustum::Frustum(const Camera& camera, const glm::vec4& viewport)
{
    float w = viewport.z;
    float h = viewport.w;

    const Projector projector(camera, viewport);

    corners_.push_back(projector.unproject(0.0f, 0.0f, 0.0f));
    corners_.push_back(projector.unproject(w,    0.0f, 0.0f));
    corners_.push_back(projector.unproject(0.0f, h,    0.0f));
    corners_.push_back(projector.unproject(w,    h,    0.0f));
    corners_.push_back(projector.unproject(0.0f, 0.0f, 1.0f));
    corners_.push_back(projector.unproject(w,    0.0f, 1.0f));
    corners_.push_back(projector.unproject(0.0f, h,    1.0f));
    corners_.push_back(projector.unproject(w,    h,    1.0f));
}

std::vector<glm::vec3> Frustum::corners() const
{
    return corners_;
}

glm::vec3 Frustum::centroid() const
{
    glm::vec3 c;

    for (int i = 0; i < 8; i++)
        c += corners_[i];

    return c * 1.0f / 8.0f;
}

glm::vec3 Frustum::farCenter() const
{
    glm::vec3 c;
    for (int i = 4; i < 8; i++)
        c += corners_[i];

    return c * 1.0f / 4.0f;
}

glm::vec3 Frustum::nearCenter() const
{
    glm::vec3 c;
    for (int i = 0; i < 4; i++)
        c += corners_[i];

    return c * 1.0f / 4.0f;
}

glm::mat4 Frustum::orthoShadowMatrix(const glm::vec3& ld,
                                     float nearClipOffset) const
{
    glm::vec3 lightDirection(ld.x, ld.y, ld.z);
    glm::vec3 frustumCentroid = centroid();
    float farDistance = glm::length(frustumCentroid - farCenter());

    // Start at the centroid, and move back in the opposite direction of the
    // light by an amount equal to the camera's farClip. This is the temporary
    // working position for the light
    glm::vec3 lightPos = frustumCentroid + lightDirection * farDistance;

    //QVector3D up = QVector3D::crossProduct( QVector3D(0,0,-1), -lightDir );
    glm::vec3 viewDir(0,0,-1);
    glm::vec3 up = glm::vec3( viewDir.x,
                             (-lightDirection.z * viewDir.z - lightDirection.x * viewDir.x) / lightDirection.y,
                             viewDir.z);
    glm::mat4 view = glm::lookAt(lightPos, frustumCentroid, up);

    // Use this view matrix to find the location of the frustum corners with
    // respect to our temporary working position (multiply the positions with
    // the view matrix)
    glm::vec3 positions[8];
    for (int i = 0; i < 8; i++)
        positions[i] = glm::vec3(view * glm::vec4(corners_[i], 1.0));

    // Loop through the 8 light-space frustum corners we just calculated, and
    // find the Min and Max X, Y, and Z.
    BoundingBox bound;
    for (int i = 0; i < 8; i++)
        bound.update(positions[i]);

    glm::vec3 min = bound.minimum();
    glm::vec3 max = bound.maximum();

    // Create an off-center orthographic projection matrix (using glOrtho)
    // based on the min and max values. Left would be min X, Right would be
    // the max X, Bottom would be min Y, Top would be max Y, near would -maxZ,
    // far would -minZ.

    glm::mat4 clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f,-1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.5f, 0.0f,
                               0.0f, 0.0f, 0.5f, 1.0f);

    glm::mat4 projection = clip * glm::ortho(
         min.x,                   max.x,
         min.y,                   max.y,
        -max.z - nearClipOffset, -min.z);


    // Use these view and projection matrices to render geometry to the
    // shadow buffer.
    return projection * view;
}

glm::mat4 Frustum::perspectiveShadowMatrix(
        const glm::vec3& lightPosition,
        const glm::vec3& lightDirection,
        const float distance,
        const float angle,
        float nearClipOffset) const
{    
    //std::cout << StringHelper::glm(lightPosition + lightDirection * distance) << std::endl;

    glm::vec3 up(0.0f, 1.0f, 0.0f);
    const glm::mat4 view = glm::lookAt(
        lightPosition,
        lightPosition - (-lightDirection),
        up);
    glm::mat4 projection =
        glm::perspective(glm::radians(angle*2), 1.0f, nearClipOffset, 50.0f);
    projection[1][1] *= -1;
    return projection * view;
}

glm::mat4 Frustum::pointLightShadowMatrix(
        const glm::vec3& lightPos,
        const float farDistance,
        const float /*nearClipOffset*/) const
{
    glm::vec3 frustumCentroid = centroid();
    //glm::vec3 up = glm::vec3( 0, 1, 0);

    glm::vec3 lightDirection = glm::normalize(frustumCentroid - lightPos);
    glm::vec3 viewDir(0,0,-1);
    glm::vec3 up = glm::vec3( viewDir.x,
                             (-lightDirection.z * viewDir.z - lightDirection.x * viewDir.x) / lightDirection.y,
                             viewDir.z);

    glm::mat4 view = glm::lookAt(lightPos, frustumCentroid, up);

    // Use this view matrix to find the location of the frustum corners with
    // respect to our temporary working position (multiply the positions with
    // the view matrix)
    glm::vec3 positions[8];
    for (int i = 0; i < 8; i++)
        positions[i] = glm::vec3(view * glm::vec4(corners_[i], 1.0));

    // Loop through the 8 light-space frustum corners we just calculated, and
    // find the Min and Max X, Y, and Z.
    BoundingBox bound;
    for (int i = 0; i < 8; i++)
        bound.update(positions[i]);

//    glm::vec3 min = bound.minimum();
//    glm::vec3 max = bound.maximum();

    // Create an off-center orthographic projection matrix (using glOrtho)
    // based on the min and max values. Left would be min X, Right would be
    // the max X, Bottom would be min Y, Top would be max Y, near would -maxZ,
    // far would -minZ.

//    glm::mat4 projection = glm::frustum(
//         min.x,                   max.x,
//         min.y,                   max.y,
//        -max.z - nearClipOffset, -min.z);

    glm::mat4 projection = glm::perspective(
            45.0f,
            1.0f,
            0.1f,
            farDistance);
    projection[1][1] *= -1;

    // Use these view and projection matrices to render geometry to the
    // shadow buffer.
    return projection * view;
}

} // namespace kuu

