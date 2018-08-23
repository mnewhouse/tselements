/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "car.hpp"
#include "handling_v2.hpp"

#include "resources/car_definition.hpp"

#include <chipmunk/chipmunk.h>

namespace ts
{
  namespace resources
  {
    class TerrainLibrary;
  }

  namespace world
  {
    Car::Car(const CarDefinition& car_definition, std::uint16_t entity_id)
      : Entity(entity_id, EntityType::Car, 
               car_definition.collision_shape, 
               car_definition.mass, 
               car_definition.moment_of_inertia),
        Controllable(entity_id),
        handling_(car_definition.handling)
    {
    }

    void Car::update(const World& world, double frame_duration)
    {
      handling_state_ = update_car_state(*this, world, frame_duration);              
    }
  }
}
