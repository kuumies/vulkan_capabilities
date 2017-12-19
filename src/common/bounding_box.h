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
#include <vector>

namespace kuu
{

/**
    A bounding box of set of points.
 **/
class BoundingBox
{
public:
    /**
        Constructs the bounding box. The bounding box is infinite.
     **/
    BoundingBox();

    /**
        Updates the bounding box to contain the point.
        \param point The point to add into bounding box.
     **/
    void update(const glm::vec3& point);

    /**
        Updates the bounding box to contain another bounding box.
        \param bb The bounding obx to add into bounding box.
     **/
    void update(const BoundingBox& bb);

    /**
        Sets the minimum point.
        \param p The minimum point.
     **/
    void setMinimum(const glm::vec3& p);

    /**
        Returns the minimum point.
        \return The minimum point of the bounding box.
     **/
    glm::vec3 minimum() const;

    /**
        Sets the maximum point.
        \param p The maximum point.
     **/
    void setMaximum(const glm::vec3& p);

    /**
        Returns the maximum point.
        \return The maximum point of the bounding box.
     **/
    glm::vec3 maximum() const;

    /**
        Returns the boundig box center point.
        \return The center point of the bounding box.
     **/
    glm::vec3 center() const;

    /**
        Returns the bounding box size.
        \return The size of bounding box.
     **/
    glm::vec3 size() const;

    /**
        Returns the bounding box corners.
        \return The corners of the bounding box.
     **/
    std::vector<glm::vec3> corners() const;

    /**
        Resets the bounding box. The box is now infinite.
     **/
    void reset();

    /**
        Returns true if the bounding box contains the given in point.
        \param p The point
        \return The contain status of point.
     **/
    bool contains(const glm::vec3& p) const;

    /**
        Compares equality with other bounding box. Boxes are equal if the min
        and max values are the same.
        \param bb The other bounding box.
        \return True if the this and the other bounding box are equal.
     **/
    bool operator==(const BoundingBox& bb) const;

    /**
        Compares unequality with other bounding box. Boxes are unequal if the
        min or max values are different.
        \param bb The other bounding box.
        \return True if the this and the other bounding box are unequal.
     **/
    bool operator!=(const BoundingBox& bb) const;
    
private:
    glm::vec3 min_;
    glm::vec3 max_;
};

} // namespace kuu
