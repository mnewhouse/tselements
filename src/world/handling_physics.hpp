/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/rotation.hpp"

namespace ts
{
  namespace resources
  {
    struct TerrainDefinition;
  }

  namespace world
  {
    class Car;

    struct CarUpdateState
    {
      Vector2<double> velocity;
      Rotation<double> rotation;
      double rotating_speed;
      double engine_rev_speed;
      double traction;
      double load_balance;
    };

    CarUpdateState update_car_state(const Car& car, const resources::TerrainDefinition& terrain, double frame_duration);
  }
}
