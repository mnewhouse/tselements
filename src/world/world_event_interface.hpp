/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "control_point_manager.hpp"

#include <cstdint>

namespace ts
{
  namespace world
  {
    class Entity;
    struct CollisionResult;

    struct EventInterface
    {
      virtual void on_collision(const Entity* entity, const CollisionResult& collision) {}
      virtual void on_collision(const Entity* subject, const Entity* object, const CollisionResult& collision) {}

      virtual void on_control_point_hit(const Entity* entity, const ControlPoint& point,
                                        std::uint32_t frame_offset) {}
    };
  }
}
