/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "car.hpp"
#include "handling_physics_detail.hpp"

namespace ts
{
  namespace world
  {
    template <typename TerrainFunc>
    void Car::update(TerrainFunc&& terrain_at, double frame_duration)
    {
      auto new_state = update_car_state(*this, std::forward<TerrainFunc>(terrain_at), frame_duration);
      handling_state_ = new_state.handling_state;

      this->set_velocity(new_state.velocity);
      this->set_rotating_speed(new_state.rotating_speed);

      this->update_z_speed(frame_duration);
    }
  }
}

