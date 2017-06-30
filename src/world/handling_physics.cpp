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
    /*
    CarUpdateState update_car_state(const Car& car, const resources::TerrainDefinition& terrain, double frame_duration)
    {
      using Control = controls::FreeControl;
      const auto acceleration_rate = car.control_state(Control::Throttle) / 255.f;
      const auto braking_rate = car.control_state(Control::Brake) / 255.f;
      const auto turning_left_rate = car.control_state(Control::Left) / 255.f;
      const auto turning_right_rate = car.control_state(Control::Right) / 255.f;
      const auto turning_rate = -turning_left_rate + turning_right_rate;

      auto rotation = car.rotation();
      auto facing_vector = transform_point({ 0.0, -1.0 }, rotation);

      auto velocity = car.velocity();     

      auto heading = normalize(velocity);
      auto speed = magnitude(velocity);

      const auto& handling = car.handling();
      const auto mass = car.mass();
      auto load_balance = car.load_balance();

      auto engine_rev_speed = car.engine_rev_speed();

      auto is_moving = std::abs(speed) >= 0.00001;

      auto traction = is_moving ? 1.0 : car.traction();
      auto oversteer_ratio = 0.0;
      auto direction = radians(std::atan2(heading.x, -heading.y));
      {
        auto oversteer_angle = normalize(direction - rotation);
        auto reverse_oversteer_angle = normalize(direction + degrees(180.0) - rotation);

        auto diff = std::min(std::abs(oversteer_angle.degrees()), std::abs(reverse_oversteer_angle.degrees()));
        auto oversteer_traction = std::max((90.0 - diff) / 90.0, 0.0);
        oversteer_ratio = 1.0 - oversteer_traction;       

        if (std::abs(engine_rev_speed) >= 0.0001)
        {
          // See how well the car's velocity matches up with the engine revs.
          auto rev_speed_traction = speed / (engine_rev_speed * handling.max_engine_revs);
          traction = std::min(traction, rev_speed_traction * oversteer_traction);
        }

        if (is_moving)
        {
          // Do the same here, except that we go the other way by flipping the division's operands.
          // We work out the minimum traction value that way.
          auto rev_speed_traction = (engine_rev_speed * handling.max_engine_revs) / speed;
          traction = std::min(traction, rev_speed_traction * oversteer_traction);
        }
      }

      auto stress_multiplier = [=](resources::Handling::StressFactor stress)
      {
        if (load_balance < 0.0)
        {
          return stress.front * -load_balance + stress.neutral * (1.0 + load_balance);
        }

        else
        {
          return stress.rear * load_balance + stress.neutral * 1.0 - load_balance;
        }
      };

      auto drag_resistance = speed * -velocity * handling.drag_coefficient;
      auto rolling_resistance = -velocity * handling.rolling_coefficient;
      auto terrain_resistance = -velocity * terrain.roughness * mass;
      auto slide_friction = -velocity * oversteer_ratio * handling.slide_friction;

      auto downforce_traction = (speed * speed * (1.0 - oversteer_ratio) * handling.downforce_coefficient) * terrain.traction;
      auto traction_limit = handling.traction_limit * terrain.traction + downforce_traction;
      auto applied_force = 0.0;

      Vector2<double> acceleration_force;
      {
        double tm = handling.torque_multiplier;
        double speed_factor = std::min(speed / handling.max_engine_revs, 1.0);
        double torque_multiplier = std::max(tm - speed_factor * (tm - 1.0), 0.5) * acceleration_rate;
        acceleration_force += handling.torque * torque_multiplier * terrain.acceleration * facing_vector;
        applied_force += handling.torque * torque_multiplier * stress_multiplier(handling.torque_stress);
      }

      Vector2<double> braking_force;

      // TODO: Think of better way to detect reversing
      if (speed != 0.0 && dot_product(heading, facing_vector) >= 0.0)
      {
        auto braking = (handling.braking + downforce_traction * handling.downforce_brake_effect) * terrain.braking;

        braking_force -= braking * braking_rate * heading;
        applied_force += braking * braking_rate * stress_multiplier(handling.braking_stress);
      }

      else
      {
        acceleration_force -= handling.torque * terrain.acceleration * braking_rate * facing_vector;
        applied_force += handling.torque * braking_rate * stress_multiplier(handling.torque_stress);          
      }

      auto base_steering = handling.steering * terrain.steering;

      auto turning_stress = stress_multiplier(handling.turning_stress);
      auto turning_stress_root = std::sqrt(turning_stress);

      auto supported_turning_force = (handling.grip + downforce_traction * handling.downforce_turning_effect);
      auto supported_turning_speed = base_steering;

      if (is_moving)
      {
        auto force_divisor = mass * speed * turning_stress_root;
        if (std::abs(force_divisor) >= 0.00001)
        {
          supported_turning_speed = std::min(supported_turning_force / force_divisor, supported_turning_speed);
        }        
      }

      auto antislide_turning_reduction = (base_steering - supported_turning_speed) * 
        std::max(handling.antislide * terrain.antislide, 1.0);
      auto absolute_turning_speed = base_steering - antislide_turning_reduction;     

      auto turning_speed = turning_rate * absolute_turning_speed;
      auto turning_force = supported_turning_force * std::abs(turning_rate);

      auto normalized_force = applied_force / traction_limit;
      auto normalized_turning_force = turning_force * turning_stress_root / traction_limit;

      auto traction_loss = std::hypot(normalized_force, normalized_turning_force);
      auto traction_ratio = 1.0;
      if (traction_loss > 1.0)
      {
        traction_ratio = 1.0 / traction_loss;
        turning_speed -= turning_rate * antislide_turning_reduction * (1.0 - traction_ratio);

        acceleration_force *= traction_ratio;
        braking_force *= traction_ratio;

        traction = std::min(traction, traction_ratio);
      }

      auto net_force = acceleration_force + braking_force + drag_resistance +
        rolling_resistance + terrain_resistance + slide_friction;

      auto new_velocity = velocity + (net_force / mass) * frame_duration;

      auto new_rotation = rotation + radians(turning_speed * frame_duration);

      auto new_facing_vector = transform_point({ 0.0, -1.0 }, new_rotation);
      auto new_speed = magnitude(new_velocity);

      auto redirect_force = make_vector2(0.0, 0.0);
      if (std::abs(new_speed) > 0.00001)
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

        // Redirect the movement based on the available grip, downforce, and turning stress level.
        auto max_redirect_force = (handling.grip + downforce_traction * handling.downforce_grip_effect) *
          terrain.grip / turning_stress_root;

        auto redirect_force_magnitude = std::min(max_redirect_force, redirect_magnitude / frame_duration * mass);
        auto redirect_direction = normalize(redirect_vector);

        redirect_force = redirect_direction * redirect_force_magnitude * traction_ratio;
      }

      auto adjustment_force = redirect_force;
      auto turning_speed_adjustment = 0.0;
      if (traction_loss > 1.0)
      {
        auto inverse_traction = 1.0 - traction_ratio;

        using resources::Handling;
        auto apply_behavior = [&](const Handling::TractionLossBehavior& behavior)
        {
          auto antislide_reduction = std::max(std::min(behavior.antislide_reduction * inverse_traction, 2.0), 0.0);
          auto turning_reduction = std::max(std::min(behavior.turning_reduction * inverse_traction, 1.0), 0.0);
          auto grip_reduction = std::max(std::min(behavior.grip_reduction * inverse_traction, 1.0), 0.0);
          auto torque_reduction = std::max(std::min(behavior.torque_reduction * inverse_traction, 1.0), 0.0);
          auto braking_reduction = std::max(std::min(behavior.braking_reduction * inverse_traction, 1.0), 0.0);

          turning_speed_adjustment += turning_rate * antislide_reduction * antislide_turning_reduction;              
          turning_speed_adjustment -= turning_reduction * turning_speed;

          adjustment_force -= grip_reduction * redirect_force;
          adjustment_force -= torque_reduction * acceleration_force;
          adjustment_force -= braking_reduction * braking_force;
        };

        auto behavior = handling.slide_behavior;
        if (braking_rate > 0.001f)
        {
          behavior = handling.lock_up_behavior;
        }

        if (acceleration_rate > 0.001f)
        {
          auto& wheel_spin_behavior = handling.wheel_spin_behavior;
          if (braking_rate > 0.001f) behavior = wheel_spin_behavior;

          // When braking *and* accelerating, make the effects accumulative.          
          behavior.antislide_reduction += wheel_spin_behavior.antislide_reduction;
          behavior.braking_reduction += wheel_spin_behavior.braking_reduction;
          behavior.grip_reduction += wheel_spin_behavior.grip_reduction;
          behavior.turning_reduction += wheel_spin_behavior.turning_reduction;
          behavior.torque_reduction += wheel_spin_behavior.torque_reduction;
        }

        apply_behavior(behavior);
      }

      {
        // Traction recovery
        auto previous_traction = car.traction();
        if (previous_traction < traction)
        {
          traction = std::min(previous_traction + handling.traction_recovery * frame_duration, traction);
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

      const auto rev_recovery = 0.5;
      {
        auto frame_recovery = rev_recovery * frame_duration;
        auto target_revs = new_speed / handling.max_engine_revs;
        if (acceleration_rate > 0.001f) target_revs /= std::max(std::sqrt(traction), 0.25);

        target_revs = std::min(target_revs, 1.1);

        if (target_revs < engine_rev_speed)
        {
          engine_rev_speed = std::max(engine_rev_speed - frame_recovery, target_revs);
        }

        else
        {
          engine_rev_speed = std::min(engine_rev_speed + frame_recovery, target_revs);
        }
      }

      CarUpdateState update_state;
      update_state.velocity = new_velocity + (adjustment_force / mass) * frame_duration;
      update_state.rotation = new_rotation + radians(turning_speed_adjustment * frame_duration);
      update_state.rotating_speed = rotating_speed;
      update_state.engine_rev_speed = engine_rev_speed;
      update_state.traction = traction;
      update_state.load_balance = load_balance;

      return update_state;
    }
    */
  }
}