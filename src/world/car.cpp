/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "car.hpp"
#include "handling_physics.hpp"

#include "resources/car_definition.hpp"

namespace ts
{
  namespace world
  {
    Car::Car(const CarDefinition& car_definition, std::uint16_t entity_id)
      : Entity(entity_id, EntityType::Car, car_definition.collision_mask),
        Controllable(entity_id),
        handling_(car_definition.handling), 
        tyre_positions_(car_definition.tyre_positions)
    {
      if (tyre_positions_.empty())
      {
        tyre_positions_.push_back({});
      }

      set_bounciness(car_definition.bounciness);
      set_mass(car_definition.handling.mass);
    }

    void Car::update(const resources::TerrainDefinition& terrain_def, double frame_duration)
    {
      auto state = update_car_state(*this, terrain_def, frame_duration);
      
      update_z_speed(frame_duration);

      set_velocity(state.velocity);
      set_rotation(state.rotation);
      set_rotating_speed(state.rotating_speed);      
      traction_ = state.traction;
      engine_rev_speed_ = state.engine_rev_speed;
      load_balance_ = state.load_balance;
    }

    const resources::Handling& Car::handling() const
    {
      return handling_;
    }

    double Car::engine_rev_speed() const
    {
      return engine_rev_speed_;
    }

    double Car::traction() const
    {
      return traction_;
    }

    double Car::load_balance() const
    {
      return load_balance_;
    }

    const boost::container::small_vector<Vector2i, 4>& Car::tyre_positions() const
    {
      return tyre_positions_;
    }
  }
}