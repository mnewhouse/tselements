/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/rotation.hpp"
#include "utility/transform.hpp"
#include "utility/interpolate.hpp"

#include "resources/handling.hpp"

#include <boost/container/small_vector.hpp>

namespace ts
{
  namespace world
  {
    class Car;

    namespace handling_v2
    {
      struct UpdateState
      {
        Vector2<double> velocity;
        double rotating_speed;

        resources::HandlingState_v2 handling_state;        
      };

      template <typename TerrainLookupFunc>
      UpdateState update_car_state(const Car& car, TerrainLookupFunc&& terrain_lookup,
                                   float frame_duration);
    }
  }
}