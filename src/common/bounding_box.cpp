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

#include "bounding_box.h"

#include <limits>
#include <glm/common.hpp>

namespace kuu
{

BoundingBox::BoundingBox()
{
    reset();
}

void BoundingBox::update(const glm::vec3& point)
{
    for (int i = 0; i < 3; ++i)
    {
        max_[i] = glm::max(point[i], max_[i]);
        min_[i] = glm::min(point[i], min_[i]);
    }
}
    
void BoundingBox::update(const BoundingBox& bb)
{
    update(bb.minimum());
    update(bb.maximum());
}

void BoundingBox::setMinimum(const glm::vec3& minimum)
{
    min_ = minimum;
}

glm::vec3 BoundingBox::minimum() const
{
    return min_;
}

void BoundingBox::setMaximum(const glm::vec3& maximum)
{
    max_ = maximum;
}

glm::vec3 BoundingBox::maximum() const
{
    return max_;
}

glm::vec3 BoundingBox::center() const
{
    return (min_ + max_) / 2.0f;
}

glm::vec3 BoundingBox::size() const
{
    return max_ - min_;
}

std::vector<glm::vec3> BoundingBox::corners() const
{
    std::vector<glm::vec3> out;
    // bottom plane
    out.push_back(min_);
    out.push_back(glm::vec3(min_.x, min_.y, max_.z));
    out.push_back(glm::vec3(max_.x, min_.y, max_.z));
    out.push_back(glm::vec3(max_.x, min_.y, min_.z));
    // top plane
    out.push_back(max_);
    out.push_back(glm::vec3(min_.x, max_.y, max_.z));
    out.push_back(glm::vec3(max_.x, max_.y, max_.z));
    out.push_back(glm::vec3(max_.x, max_.y, min_.z));
    return out;
}

void BoundingBox::reset()
{
    min_ = glm::vec3(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max());

    max_ = glm::vec3(
        -std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity());
}

bool BoundingBox::contains(const glm::vec3& p) const
{
    if (p.x < min_.x || p.x > max_.x)
        return false;

    if (p.y < min_.y || p.y > max_.y)
        return false;

    if (p.z < min_.z || p.z > max_.z)
        return false;

    return true;
}

bool BoundingBox::operator==(const BoundingBox& bb) const
{
    return bb.minimum() == min_ && bb.maximum() == max_;
}

bool BoundingBox::operator!=(const BoundingBox& bb) const
{
    return !operator==(bb);
}

} // namespace kuu
