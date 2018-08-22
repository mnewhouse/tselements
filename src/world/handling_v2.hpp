/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>

#include "utility/vector2.hpp"
#include "utility/color.hpp"

#include <boost/container/small_vector.hpp>

namespace ts
{
  namespace world
  {
    class Car;
    class World;

    struct HandlingState
    {
      std::int8_t current_gear = 0;
      std::int8_t gear_shift_state = 0;
      double engine_rev_speed = 0.0;
      Vector2d net_force;

      struct WheelState
      {
        Vector2d pos;
        double speed;
        double slide_ratio;
        double terrain_roughness = 0.0;        
        Colorb terrain_color;
      };

      boost::container::small_vector<WheelState, 4> wheel_states;      
    };

    HandlingState update_car_state(Car& car, const World& world, double frame_duration);

    //HandlingState apply_physics_forces(Car& car, const TerrainMap& terrain_map,
    //                                   double frame_duration);
  }
}