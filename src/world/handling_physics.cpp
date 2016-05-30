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
    CarUpdateState update_car_state(const Car& car, const resources::TerrainDefinition& terrain, double frame_duration)
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

      auto actual_traction = 1.0;
      auto direction = radians(std::atan2(heading.x, -heading.y));
      {
        auto oversteer_angle = normalize(direction - rotation);
        auto reverse_oversteer_angle = normalize(direction + degrees(180.0) - rotation);

        auto diff = std::min(std::abs(oversteer_angle.degrees()), std::abs(reverse_oversteer_angle.degrees()));
        actual_traction = std::max((90.0 - diff) / 90.0, 0.0);
      }
      auto oversteer_ratio = 1.0 - actual_traction;

      const auto& handling = car.handling();
      const auto mass = car.mass();
      auto load_balance = car.load_balance();

      auto stress_multiplier = [=](resources::Handling::StressFactor stress)
      {
        if (load_balance < 0.0)
        {
          return stress.front * -load_balance + stress.neutral * (1.0 + load_balance);
        }

        else
        {
          return stress.back * load_balance + stress.neutral * 1.0 - load_balance;
        }
      };

      double traction_limit = handling.traction_limit + handling.downforce_coefficient * speed;
      double applied_force = 0.0;

      Vector2<double> acceleration_force;
      if (is_accelerating)
      {
        double tm = handling.torque_multiplier;
        double torque_multiplier = std::max(tm - car.engine_rev_speed() * (tm - 1.0), 0.5);
        acceleration_force += handling.torque * torque_multiplier * terrain.acceleration * facing_vector;
        applied_force += handling.torque * torque_multiplier * stress_multiplier(handling.torque_stress);
      }

      Vector2<double> braking_force;
      if (is_braking)
      {
        // TODO: Think of way to detect reversing
        if (speed != 0.0 && dot_product(heading, facing_vector) >= 0.0)
        {
          braking_force -= handling.braking * terrain.braking * heading;
          applied_force += handling.braking * stress_multiplier(handling.braking_stress);
        }

        else
        {
          acceleration_force -= handling.torque * terrain.acceleration * facing_vector;
          applied_force += handling.torque * stress_multiplier(handling.torque_stress);          
        }
      }

      auto actual_steering = handling.steering * terrain.steering;
      auto antislide_steering_reduction = 0.0;
      
      auto traction_available_for_steering = std::max(traction_limit - applied_force, traction_limit * 0.5);
      if (speed != 0.0)
      {        
        auto current_steering = std::min(traction_available_for_steering * handling.grip * 
                                         terrain.steering / (mass * speed), actual_steering);
        
        antislide_steering_reduction = (actual_steering - current_steering) * (handling.antislide * terrain.grip);
        actual_steering -= antislide_steering_reduction;
      }

      double turning_speed = 0.0;
      if (is_turning_left)
      {
        turning_speed -= actual_steering;
      }

      if (is_turning_right)
      {
        turning_speed += actual_steering;
      }
      
      auto drag_resistance = speed * speed * -heading * handling.drag_coefficient;
      auto rolling_resistance = -velocity * handling.rolling_coefficient;
      auto terrain_resistance = -velocity * terrain.roughness * mass; 
      auto slide_friction = -heading * oversteer_ratio * mass * 5.0;

      auto net_force = acceleration_force + braking_force + drag_resistance +
        rolling_resistance + terrain_resistance + slide_friction;

      auto new_velocity = velocity + (net_force / mass) * frame_duration;

      auto new_rotation = rotation + radians(turning_speed * frame_duration);

      Vector2<double> adjustment_force, redirect_force;
      double steering_adjustment = 0.0;

      auto new_facing_vector = transform_point({ 0.0, -1.0 }, new_rotation);
      auto new_speed = magnitude(new_velocity);

      auto turning_force = 0.0;
      if (is_turning_left != is_turning_right) turning_force = traction_available_for_steering;
      applied_force += turning_force * handling.antislide;

      if (new_speed != 0.0)
      {
        // Adjust the car's velocity according to its new rotation.
        // If the car goes over the limit of what the tyres can take, the car will slide.
        auto new_heading = normalize(new_velocity);

        // Decrease steering according to the antislide property.
        auto target_velocity = new_facing_vector * new_speed;

        // These variables represent the amount the car needs to accelerate in one frame
        // in order for the heading to match up with the rotation.
        auto redirect_vector = (target_velocity - new_velocity);
        auto reverse_redirect_vector = (-target_velocity - new_velocity);

        auto redirect_magnitude = magnitude(redirect_vector);
        auto reverse_redirect_magnitude = magnitude(reverse_redirect_vector);

        if (reverse_redirect_magnitude < redirect_magnitude)
        {
          redirect_vector = reverse_redirect_vector;
          redirect_magnitude = reverse_redirect_magnitude;
        }

        auto max_redirect_force = traction_limit * handling.grip * terrain.grip;
        auto redirect_force_magnitude = std::min(max_redirect_force, redirect_magnitude / frame_duration * mass);
        auto redirect_direction = normalize(redirect_vector);
        redirect_force = redirect_direction * redirect_force_magnitude;

        // Now, use all the available traction force to change the car's direction.
        // So how much traction is available for steering?

        adjustment_force += redirect_force;
      }

      {
          const auto& lock_up_behavior = handling.lock_up_behavior;
          const auto& wheel_spin_behavior = handling.wheel_spin_behavior;
          const auto& slide_behavior = handling.slide_behavior;

          if (applied_force > traction_limit)
          {
            auto ratio = traction_limit / applied_force;
            ratio *= ratio;

            actual_traction *= ratio;
            auto inverse_ratio = 1.0 - actual_traction;

            //steering_adjustment -= turning_speed * inverse_ratio;
            adjustment_force -= (acceleration_force + braking_force) * inverse_ratio;

            redirect_force *= ratio;
            turning_speed *= ratio;
            acceleration_force *= ratio;
            braking_force *= ratio;

            using resources::Handling;
            auto apply_behavior = [&](const Handling::TractionLossBehavior& behavior)
            {
              const double reduction[] =
              {
                std::max(std::min(behavior.antislide_reduction * inverse_ratio, 1.0), 0.0),
                std::max(std::min(behavior.steering_reduction * inverse_ratio, 1.0), 0.0),
                std::max(std::min(behavior.turn_in_reduction * inverse_ratio, 1.0), 0.0),
                std::max(std::min(behavior.torque_reduction * inverse_ratio, 1.0), 0.0),
                std::max(std::min(behavior.braking_reduction * inverse_ratio, 1.0), 0.0)
              };

              if (is_turning_left) steering_adjustment -= reduction[0] * antislide_steering_reduction;
              if (is_turning_right) steering_adjustment += reduction[0] * antislide_steering_reduction;
              
              steering_adjustment -= reduction[1] * turning_speed;
              adjustment_force -= reduction[2] * redirect_force;
              adjustment_force -= reduction[3] * acceleration_force;
              adjustment_force -= reduction[4] * braking_force;
            };

            if (is_braking)
            {
              apply_behavior(lock_up_behavior);
            }

            else if (is_accelerating)
            {
              apply_behavior(wheel_spin_behavior);
            }

            else
            {
              apply_behavior(slide_behavior);
            }
        }
      }

      {
        const double traction_recovery = 1.0;

        // Traction recovery
        auto previous_traction = car.traction();
        if (previous_traction < actual_traction)
        {
          actual_traction = std::min(previous_traction + traction_recovery * frame_duration, actual_traction);
        }
      }

      {
        auto net_force_1d = dot_product(net_force, new_facing_vector);
        auto shift_towards = 0.0;
        if (handling.load_balance_limit != 0.0)
        {
          shift_towards = std::max(std::min(net_force_1d / handling.load_balance_limit, 1.0), -1.0);
        }

        auto shift_factor = handling.balance_shift_factor * frame_duration;
        load_balance = load_balance + (shift_towards - load_balance) * shift_factor;
      }

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

      CarUpdateState update_state;
      update_state.velocity = new_velocity + (adjustment_force / mass) * frame_duration;
      update_state.rotation = new_rotation + radians(steering_adjustment * frame_duration);
      update_state.rotating_speed = rotating_speed;
      update_state.engine_rev_speed = magnitude(new_velocity) / handling.max_engine_revs;
      update_state.traction = actual_traction;
      update_state.load_balance = load_balance;

      return update_state;
    }
  }
}