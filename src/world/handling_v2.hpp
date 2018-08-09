/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>

#include "utility/vector2.hpp"

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
      double load_transfer = 0.0;
      double engine_rev_speed = 0.0;      
    };

    struct UpdateState
    {
      HandlingState handling_state;
      Vector2d velocity;
      double angular_velocity;
    };

    HandlingState apply_physics_forces(Car& car, const TerrainMap& terrain_map,
                                     double frame_duration);

    //HandlingState apply_physics_forces(Car& car, const TerrainMap& terrain_map,
    //                                   double frame_duration);
  }
}