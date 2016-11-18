/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>

namespace ts
{
  namespace world
  {
    class Entity;
  }

  namespace stage
  {
    namespace messages
    {
      struct LapComplete
      {
        const world::Entity* entity;
        std::uint32_t lap_time;
        std::uint32_t race_time;
      };

      struct SectorComplete
      {
        const world::Entity* entity;
        std::uint32_t sector_id;
        std::uint32_t sector_time;
        std::uint32_t lap_time;
        std::uint32_t race_time;
      };

      struct RaceTimeUpdate
      {
        std::uint32_t old_race_time;
        std::uint32_t new_race_time;
      };
    }
  }
}
