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
#include <iostream>

namespace kuu
{

/**
    A ray in three-dimensional space.
 **/
class Ray
{
public:
    /**
        Constructs a null ray.
     **/
    Ray();

    /**
        Constructs ray from the data.
        \param start The start position of the ray.
        \param direction The direction of the ray.
     **/
    Ray(const glm::vec3& start, const glm::vec3& direction);

    /**
        Returns true if the ray is a null ray (start and direction are null
        vectors).
        \return The null status.
     **/
    bool isNull() const;

    /**
        Sets the ray starting position.
        \param pos The starting position of the ray.
     **/
    void setStart(const glm::vec3& start);

    /**
        Returns the starting position.
        \return The starting position of the ray.
     **/
    glm::vec3 start() const;

    /**
        Sets the ray direction.
        \param direction The direction of the ray.
     **/
    void setDirection(const glm::vec3& direction);

    /**
        Returns the direction.
        \return The direction of the ray.
     **/
    glm::vec3 direction() const;

    /**
        Returns the position along the ray at the given in distance.
        \param distance The distance to to travel from the start along
            with the ray.
        \return The position.
     **/
    glm::vec3 position(float distance) const;

    /**
        Compares equality with another ray. Rays are equal if the have the
        same start and direction.

        \param ray The other ray.
        \return Returns true if the rays are equal.
     **/
    bool operator==(const Ray& ray) const;

    /**
        Compares unequality with another ray. Rays are unequal if the have
        different starts and/or directions.

        \param ray The other ray.
        \return Returns true if the rays are unequal.
     **/
    bool operator!=(const Ray& ray) const;

private:
    glm::vec3 start_;
    glm::vec3 direction_;
};

} // namespace kuu
