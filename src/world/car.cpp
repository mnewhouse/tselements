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
        handling_state_()
    {
      set_bounciness(car_definition.bounciness);
      set_mass(car_definition.handling.mass);
    }

    const resources::Handling& Car::handling() const
    {
      return handling_;
    }

    const resources::HandlingState& Car::handling_state() const
    {
      return handling_state_;
    }
  }
}
