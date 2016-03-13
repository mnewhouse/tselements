/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef WORLD_MESSAGES_HPP_66289811981
#define WORLD_MESSAGES_HPP_66289811981

#include "collisions.hpp"

#include <cstdint>

namespace ts
{
  namespace world
  {
    class Entity;

    namespace messages
    {
      struct ControlPointHit
      {
        const Entity* entity;
        std::uint16_t point_id;
        std::uint32_t frame_offset;
      };

      struct SceneryCollision
      {
        const Entity* entity;
        CollisionResult collision;
      };

      struct EntityCollision
      {
        const Entity* subject;
        const Entity* object;
        CollisionResult collision;
      };
    }
  }
}

#endif