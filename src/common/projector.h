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

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include "camera.h"
#include "ray.h"

namespace kuu
{

struct Camera;

/**
    Project a point from the world space into the viewport space and vice-versa.
 **/
class Projector
{
public:
    /**
        Constructs the projector from camera.
        \param camera Camera that is used with projection and unprojection.
                      The camera must contain a valid camera matrix and viewport
                      and cannot be null pointer.
     **/
    Projector(const Camera& camera,
              const glm::vec4& viewport);

    /**
        Projects a world space point into camera viewport.
        \param point The world space point.
        \return Returns the viewport coordinate of the projected point.
                The point is a null point if the projection failed.
     **/
    glm::vec2 project(const glm::vec3& point) const;

    /**
        Projects a world space point into camera viewport.
        \param x The world space point x component.
        \param y The world space point y component.
        \param z The world space point z component.
        \return Returns the viewport coordinate of the projected point.
                The point is a null point if the projection failed.
     **/
    glm::vec2 project(float x, float y, float z) const;

    /**
        Unprojects a viewport point into world space.
        \param point The viewport space point. The z-value should be at the
                     range of [0.0, 1.0] where 0.0 matches the near viewplane
                     and 1.0 matches the far view plane.
        \param topDown Set true if the [0, 0] viewport coordinate is located at
                       top-left corner.
        \return The world space coordinate of unprojected point. The point is a
                null point if the unprojection failed.
     **/
    glm::vec3 unproject(const glm::vec3& point, bool topDown = true) const;

    /**
        Unprojects a viewport point into world space.
        \param x The viewport point x component.
        \param y The viewport point y component.
        \param z The viewport point z component.
        \param topDown Set true if the [0, 0] viewport coordinate is located at
                       top-left corner.
        \return The world space coordinate of unprojected point. The point is a
                null point if the unprojection failed.
     **/
    glm::vec3 unproject(float x, float y, float z, bool topDown = true) const;

    /**
        Creates a world space ray from the viewport coordinate that starts
        from the near plane and moves towards the far plane.
        \param pos The viewport coordinate. The coordinate must be positive.
        \return Returns the viewport ray.
     **/
    Ray viewportRay(const glm::vec2& pos) const;

private:
    const Camera camera_;
    const glm::vec4 viewport_;
};

} // namespace kuu
