/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>

#include "resources/handling.hpp"

#include "utility/vector2.hpp"

namespace ts
{
  namespace world
  {
    class Car;
    class Entity;

    namespace messages
    {
      struct ControlPointHit
      {
        const Entity* entity;
        std::uint16_t point_id;
        std::uint32_t point_flags;
        std::uint32_t frame_offset;
      };

      struct SceneryCollision
      {
        const Entity* entity;
      };

      struct EntityCollision
      {
        const Entity* subject;
        const Entity* object;
      };

      struct CarPropertiesUpdate
      {
        const Car* car;
        double mass;
        Vector2d center_of_mass;
        double moment;
        resources::Handling handling;
      };
    }
  }
}
