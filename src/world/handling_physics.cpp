/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "handling_physics.hpp"
#include "car.hpp"

#include "resources/terrain_definition.hpp"

#include "utility/transform.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>

namespace ts
{
  namespace world
  {
    void update_car_state(Car& car, const resources::TerrainDefinition& terrain, double frame_duration)
    {
      using controls::Control;
      const bool is_accelerating = car.control_state(Control::Accelerate);
      const bool is_braking = car.control_state(Control::Brake);
      const bool is_turning_left = car.control_state(Control::Left);
      const bool is_turning_right = car.control_state(Control::Right);

      auto rotation = car.rotation();
      auto facing_vector = transform_point({ 0.0, -1.0 }, rotation);

      auto velocity = car.velocity();
      auto heading = normalize(velocity);
      auto speed = magnitude(velocity);

      auto actual_traction = speed != 0.0 ? std::abs(dot_product(heading, facing_vector)) : 1.0;

      /*
      double gear_ratio = 1.0;
      auto torque_multiplier = 1.0 - (car.engine_rev_speed_ - 0.75) * (car.engine_rev_speed_ - 0.75);
      torque_multiplier *= gear_ratio;
      */

      const double steering = 1.85;
      const double viscosity = 0.8;
      const double braking = 45000.0;
      const double torque = 21000.0;
      const double max_engine_revs = 300.0;
      const double mass = 500.0;
      const double drag_coefficient = 0.3;
      const double rolling_resistance_coefficient = 50.0;

      const double grip = 65000;
      const double turn_in = 50000;
      const double antislide = 0.6;
      const double traction_limit = 80000;

      double applied_force = 0.0;

      Vector2<double> acceleration_force;
      if (is_accelerating)
      {
        double torque_multiplier = std::max(3.0 - car.engine_rev_speed_ * 2.0, 0.5);
        acceleration_force += torque * torque_multiplier * facing_vector * terrain.acceleration;
        applied_force += torque * torque_multiplier;
      }

      Vector2<double> braking_force;
      if (is_braking)
      {
        // TODO: Think of way to detect reversing
        if (speed != 0.0 && dot_product(heading, facing_vector) >= 0.0)
        {
          braking_force -= braking * heading;
          applied_force += braking;
        }

        else
        {
          acceleration_force -= torque * facing_vector * terrain.acceleration;
          applied_force += torque;
        }
      }


      auto current_steering = std::min(turn_in / (mass * speed), steering) * terrain.steering;
      auto actual_steering = steering - (steering - current_steering) * (antislide * terrain.grip);

      double turning_speed = 0.0;
      if (is_turning_left)
      {
        turning_speed -= actual_steering;
      }

      if (is_turning_right)
      {
        turning_speed += actual_steering;
      }
      
      auto drag_resistance = speed * speed * -heading * drag_coefficient;
      auto rolling_resistance = -velocity * rolling_resistance_coefficient;
      auto terrain_resistance = -velocity * terrain.roughness * mass;

      auto net_force = acceleration_force + braking_force + drag_resistance + 
        rolling_resistance + terrain_resistance;

      auto new_velocity = velocity + (net_force / mass) * frame_duration;

      auto new_rotation = rotation + radians(turning_speed * frame_duration);

      Vector2<double> adjustment_force = {};
      double steering_adjustment = 0.0;

      {
        // Adjust the car's velocity according to its new rotation.
        // If the car goes over the limit of what the tyres can take, the car will slide.
        auto new_heading = normalize(new_velocity);
        auto new_facing_vector = transform_point({ 0.0, -1.0 }, new_rotation);
        auto new_speed = magnitude(new_velocity);

        // Decrease steering according to the antislide property.

        auto target_velocity = new_facing_vector * new_speed;

        // These variables represent the amount the car needs to accelerate in one frame
        // in order for the heading to match up with the rotation.
        auto redirect_vector = (target_velocity - new_velocity);
        auto reverse_redirect_vector = (-target_velocity - new_velocity);

        auto redirect_magnitude = magnitude(redirect_vector);
        auto reverse_redirect_magnitude = magnitude(reverse_redirect_vector);

        const auto max_redirect_force = grip * terrain.grip;
        if (reverse_redirect_magnitude < redirect_magnitude)
        {
          redirect_vector = reverse_redirect_vector;
          redirect_magnitude = reverse_redirect_magnitude;
        }

        auto redirect_force_magnitude = std::min(max_redirect_force, redirect_magnitude / frame_duration * mass);

        auto redirect_direction = normalize(redirect_vector);

        auto redirect_force = redirect_direction * redirect_force_magnitude;

        // Now, use all the available traction force to change the car's direction.

        adjustment_force += redirect_force;   
        applied_force += redirect_force_magnitude;        

        if (applied_force > traction_limit)
        {
          auto ratio = traction_limit / applied_force;

          if (is_braking)
          {
            adjustment_force -= 0.4 * redirect_force * (1.0 - ratio);
            steering_adjustment -= std::min(0.8 * (1.0 - ratio), 1.0) * turning_speed;
          }

          else if (is_accelerating)
          {
            adjustment_force -= 1.0 * redirect_force * (1.0 - ratio);
            steering_adjustment -= -0.5 * turning_speed * (1.0 - ratio);
          }

          //if (is_braking) adjustment_force += 1.0 * braking_force * (1.0 - ratio);
          //if (is_accelerating) adjustment_force -= 1.0 * acceleration_force * (1.0 - ratio);
        }
      }

      // Make sure we don't accelerate through the engine rev limit (1.0).
      car.engine_rev_speed_ = magnitude(new_velocity) / max_engine_revs;
      car.traction_ = actual_traction;

      car.set_velocity(new_velocity + (adjustment_force / mass) * frame_duration);
      car.set_rotation(new_rotation + radians(steering_adjustment * frame_duration));

      // Rotational dampening

      const auto spin_recovery = 5.0;
      auto rotating_speed = car.rotating_speed();
      if (rotating_speed < 0)
      {
        rotating_speed = std::min(rotating_speed + spin_recovery * frame_duration, 0.0);
      }

      else
      {
        rotating_speed = std::max(rotating_speed - spin_recovery * frame_duration, 0.0);
      }

      car.set_rotating_speed(rotating_speed);
    }
  }
}