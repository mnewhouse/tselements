/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/rotation.hpp"

#include "resources/handling.hpp"

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
      double turning_speed = 0.0;
      double load_balance = 0.0;
    };

    struct CarUpdateState
    {
      Vector2d velocity = {};
      double rotating_speed = 0.0;

      HandlingState handling_state;
    };

    CarUpdateState update_car_state(const Car& car, const TerrainMap& terrain_map, double frame_duration);
  }
}
