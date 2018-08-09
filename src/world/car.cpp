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
  namespace world
  {
    static void update_velocity(cpBody* body, cpVect gravity, cpFloat damping, cpFloat dt)
    {
      auto angular_damping = 1.2;
      auto ang_vel = cpBodyGetAngularVelocity(body);
      ang_vel -= ang_vel * angular_damping * dt;
      cpBodySetAngularVelocity(body, ang_vel);

      cpBodyUpdateVelocity(body, gravity, damping, dt);
    }

    Car::Car(const CarDefinition& car_definition, std::uint16_t entity_id)
      : Entity(entity_id, EntityType::Car, 
               car_definition.collision_shape, 
               car_definition.mass, 
               car_definition.moment_of_inertia),
        Controllable(entity_id)
    {
      auto body = static_cast<cpBody*>(physics_body());
      cpBodySetVelocityUpdateFunc(body, update_velocity);
    }

    void Car::update(const TerrainMap& terrain_map, double frame_duration)
    {
      handling_state_ = apply_physics_forces(*this, terrain_map, frame_duration);              
    }
  }
}
