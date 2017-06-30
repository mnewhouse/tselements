/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/rotation.hpp"

#include "resources/handling.hpp"

namespace ts
{
  namespace resources
  {
    struct TerrainDefinition;
  }

  namespace world
  {
    class Car;

    struct CarUpdateState_v0
    {
      Vector2<double> velocity = {};
      Rotation<double> rotation = {};
      double rotating_speed = 0.0;      

      resources::HandlingState handling_state;
    };

    struct CarUpdateState
    {
      Vector2<double> velocity = {};
      double rotating_speed = 0.0;

      resources::HandlingState handling_state;
    };

    template <typename TerrainFunc>
    CarUpdateState update_car_state(const Car& car, TerrainFunc&& terrain_at, double frame_duration);
  }
}
