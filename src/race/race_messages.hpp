/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RACE_MESSAGES_HPP_914128935
#define RACE_MESSAGES_HPP_914128935

#include <cstdint>

namespace ts
{
  namespace world
  {
    class Entity;
  }

  namespace race
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
    }
  }
}

#endif