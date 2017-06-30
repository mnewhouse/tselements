/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "handling_physics.hpp"
#include "car.hpp"

#include "utility/transform.hpp"
#include "utility/math_utilities.hpp"
#include "utility/interpolate.hpp"

#include <boost/container/small_vector.hpp>

#include <cmath>

namespace ts
{
  namespace world
  {
    template <typename TerrainFunc>
    CarUpdateState update_car_state(const Car& car, TerrainFunc&& terrain_at, double frame_duration)
    {
      using Terrain = resources::TerrainDefinition;

      CarUpdateState result;

      using Control = controls::FreeControl;
      auto throttle_rate = car.control_state(Control::Throttle) / 255.0;
      auto braking_rate = car.control_state(Control::Brake) / 255.0;
      auto turning_left_rate = car.control_state(Control::Left) / 255.0;
      auto turning_right_rate = car.control_state(Control::Right) / 255.0;
      auto turning_rate = -turning_left_rate + turning_right_rate;

      const auto& handling = car.handling();
      auto& handling_state = result.handling_state = car.handling_state();

      auto abs_turning_rate = std::abs(turning_rate);

      auto position = car.position();
      auto rotation = car.rotation();
      auto rotating_speed = car.rotating_speed();
      auto facing_vector = transform_point({ 0.0, -1.0 }, rotation);
      auto transform = make_transformation(rotation);
      auto inverse_transform = make_transformation(-rotation);

      auto velocity = car.velocity();
      auto heading = normalize(velocity);
      auto speed = magnitude(velocity);
      auto heading_angle = radians(std::atan2(velocity.x, -velocity.y));

      const Terrain& terrain = terrain_at(position);

      const bool is_moving = std::abs(speed) >= 0.00001;

      auto old_gear = handling_state.current_gear;

      if (!is_moving)
      {
        handling_state.current_gear = 0;
      }

      const bool is_reversing = (!is_moving && braking_rate > throttle_rate) ||
        handling_state.current_gear < 0;

      auto gear_ratio = 1.0;

      if (is_reversing)
      {
        // If car is reversing, switch controls for acceleration and braking.
        std::swap(throttle_rate, braking_rate);

        handling_state.current_gear = -1;
        gear_ratio = -handling.reverse_gear_ratio;
      }

      auto torque_multiplier = 1.0;
      if (old_gear != handling_state.current_gear || handling_state.engine_rev_speed > 1.0)
      {
        torque_multiplier = 0.0;
      }

      else if (!is_reversing)
      {
        auto extra_torque = (1.0 - handling_state.engine_rev_speed) * handling.extra_torque;
        torque_multiplier = 1.0 + extra_torque;
      }

      auto braking_direction = -heading;

      // oversteer_ratio: 1 = no oversteer, 0 = fully sideways
      auto oversteer_ratio = dot_product(heading, facing_vector);

      // Square the thing, to get the right proportions.
      oversteer_ratio *= oversteer_ratio;

      auto load_balance = handling_state.load_balance;
      auto mass = static_cast<double>(handling.mass);

      auto front_load = static_cast<double>(handling.mass_distribution);
      auto rear_load = 1.0 - front_load;

      // Calculate drag resistance for downforce. The actual drag effect will be calculated
      // based on the car's new velocity.
      auto drag_resistance = -velocity * speed * handling.drag_coefficient;
      auto downforce = magnitude(drag_resistance) * oversteer_ratio * handling.downforce_coefficient;

      auto acceleration_force_1d = handling.torque * torque_multiplier * gear_ratio * throttle_rate;
      auto braking_force_1d = (handling.braking + downforce * handling.downforce_effect.braking) * braking_rate;

      auto movement_angle = radians(std::atan2(velocity.x, -velocity.y));
      auto slip_angle = rotation - movement_angle;
      if (dot_product(velocity, facing_vector) < 0.0)
      {
        slip_angle -= degrees(180.0);
      }
      slip_angle.normalize();

      auto slip_radians = std::abs(slip_angle.radians());
      auto turning_speed = handling.steering * terrain.steering * abs_turning_rate;

      auto cornering_force = handling.cornering + downforce * handling.downforce_effect.cornering;
      auto antislide_force = handling.antislide + downforce * handling.downforce_effect.antislide;

      // If the cornering force would cause the turning speed to be more than turning_speed,
      // then lower the cornering force.      
      if (terrain.cornering >= 0.0001)
      {
        auto full_steering_cornering_force = std::abs(turning_speed * speed * mass / terrain.cornering);       
        cornering_force = std::min(full_steering_cornering_force, cornering_force);
      }      

      // Calculate the antislide force that's required to make the car stop sliding completely.
      if (terrain.antislide > 0.0001)
      {
        auto max_antislide_force = slip_radians * mass * speed / (frame_duration * terrain.antislide);
        antislide_force = std::min(max_antislide_force, antislide_force);
      }

      // ************ Limit control input based on traction limit.
      auto traction_limit = handling.traction_limit + downforce * handling.downforce_effect.traction_limit;

      auto lateral_stress = (cornering_force * handling.stress_factor.cornering + 
                             antislide_force * handling.stress_factor.antislide) * 0.5;

      auto longitudinal_stress = std::abs(acceleration_force_1d * handling.stress_factor.torque) -
        std::abs(braking_force_1d * handling.stress_factor.braking);
            
      auto stress_magnitude = std::hypot(lateral_stress, longitudinal_stress);  

      if (stress_magnitude > traction_limit)
      {
        auto traction_ratio = traction_limit / stress_magnitude;
        auto stress_ratio = 1.0 - traction_ratio;       

        stress_magnitude -= stress_magnitude * std::min(handling.input_moderation * stress_ratio, 1.0);

        auto steering_bias = std::min(handling.steering_bias, 1.0f);

        auto adjusted_lateral_stress = std::min(lateral_stress, stress_magnitude * steering_bias);
        auto adjusted_longitudinal_stress = std::sqrt(stress_magnitude * stress_magnitude -
                                                      adjusted_lateral_stress * adjusted_lateral_stress);

        if (adjusted_lateral_stress < lateral_stress)
        {
          auto ratio = adjusted_lateral_stress / lateral_stress;
          cornering_force *= ratio;
          antislide_force *= ratio;
        }

        if (adjusted_longitudinal_stress < longitudinal_stress)
        {
          auto ratio = adjusted_longitudinal_stress / longitudinal_stress;
          braking_force_1d *= ratio;
          acceleration_force_1d *= ratio;
        }
      }

      if (stress_magnitude > traction_limit)
      {
        auto multiplier = traction_limit / stress_magnitude;

        cornering_force *= multiplier;
        antislide_force *= multiplier;
        braking_force_1d *= multiplier;
        acceleration_force_1d *= multiplier;
      }      

      auto transformed_velocity = velocity;

      if (is_moving)
      {
        auto redirect_angle = antislide_force / (mass * speed) * (frame_duration * terrain.antislide);
        if (slip_angle.radians() < 0.0) redirect_angle = -redirect_angle;

        transformed_velocity = transform_point(velocity, radians(redirect_angle));

        turning_speed = cornering_force * terrain.cornering / (mass * speed);

        if (turning_speed >= 0.0001 && handling.min_turning_radius >= 0.001)
        {
          turning_speed = std::min(turning_speed, speed / handling.min_turning_radius);
        }        
      }      

      // We have the input parameters now, so we can apply the throttle/braking/steering.

      auto net_force = acceleration_force_1d * facing_vector - braking_force_1d * heading;
      auto force_multiplier = frame_duration / handling.mass;

      if (turning_rate < 0.0) turning_speed = -turning_speed;

      // Apply the acceleration force
      auto new_velocity = transformed_velocity + net_force * force_multiplier;
      auto new_heading = normalize(new_velocity);

      // Then with the new velocity, calculate the drag resistance and apply that.
      drag_resistance = -new_velocity * magnitude(new_velocity) * handling.drag_coefficient;   

      auto rolling_resistance = -heading * (handling.rolling_resistance + 
        downforce * handling.downforce_effect.rolling_resistance);
      new_velocity += (drag_resistance + rolling_resistance) * force_multiplier;

      result.velocity = new_velocity;
      result.rotating_speed = rotating_speed + (turning_speed - handling_state.turning_speed);

      handling_state.turning_speed = turning_speed;      

      const auto max_engine_revs = 1.08;
      const auto engine_rev_up_rate = 3.0;
      const auto engine_rev_down_rate = 3.0;

      auto new_engine_revs = std::abs(magnitude(new_velocity) * gear_ratio / handling.max_engine_revs);
      handling_state.engine_rev_speed = static_cast<float>(new_engine_revs);      

      // If the velocity changes sign, set it to zero to prevent resistances to cause the speed
      // to go through zero.
      if (is_moving && dot_product(result.velocity, velocity) < 0.0)
      {
        result.velocity = {};
      }

      {        
        auto delta_v = transform_point(result.velocity - velocity, inverse_transform);

        auto g = magnitude(delta_v / frame_duration);

        auto target_balance = delta_v.y / frame_duration * handling.mass;
        auto diff = target_balance - handling_state.load_balance;

        handling_state.load_balance += diff * handling.load_transfer * frame_duration;
      }

      return result;
    }
  }
}

