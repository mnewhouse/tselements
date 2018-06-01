/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>

namespace ts
{
  namespace world
  {
    class Car;
    class TerrainMap;

    struct HandlingState
    {
      std::int8_t current_gear = 0;
      std::int8_t gear_shift_state = 0;
      double engine_rev_speed = 0.0;
    };
      
    HandlingState apply_physics_forces(Car& car, const TerrainMap& terrain_map,
                                       double frame_duration);
  }
}