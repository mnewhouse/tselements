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
        Controllable(entity_id)
    {
      set_bounciness(car_definition.bounciness);
      set_mass(400.0);
    }

    void Car::update(const resources::TerrainDefinition& terrain_def, double frame_duration)
    {
      update_car_state(*this, terrain_def, frame_duration);
    }
  }
}