/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "car.hpp"
#include "handling_v2.hpp"

#include "resources/car_definition.hpp"

namespace ts
{
  namespace world
  {
    Car::Car(const CarDefinition& car_definition, std::uint16_t entity_id)
      : Entity(entity_id, EntityType::Car, 
               car_definition.collision_shape, 
               car_definition.mass, 
               car_definition.moment_of_inertia),
        Controllable(entity_id)
    {      
    }

    void Car::update(const TerrainMap& terrain_map, double frame_duration)
    {
      apply_physics_forces(*this, terrain_map, frame_duration);
    }
  }
}
