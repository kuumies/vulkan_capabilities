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
 
#include "ray.h"

namespace kuu
{

Ray::Ray()
{}

Ray::Ray(const glm::vec3& start, const glm::vec3& direction)
    : start_(start)
    , direction_(direction)
{}

bool Ray::isNull() const
{
    return start_     == glm::vec3(0.0) &&
           direction_ == glm::vec3(0.0);
}

void Ray::setStart(const glm::vec3& start)
{
    start_ = start;
}

glm::vec3 Ray::start() const
{
    return start_;
}

void Ray::setDirection(const glm::vec3& direction)
{
    direction_ = direction;
}

glm::vec3 Ray::direction() const
{
    return direction_;
}

glm::vec3 Ray::position(float distance) const
{
    return start_ + direction_ * distance;
}

bool Ray::operator==(const Ray& r) const
{
    return start_ == r.start() && direction_ == r.direction();
}

bool Ray::operator!=(const Ray& r) const
{
    return !operator==(r);
}

} // namespace kuu
