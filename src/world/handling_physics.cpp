/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

/*
#include "handling_physics.hpp"
#include "car.hpp"

#include "resources/terrain_definition.hpp"

#include "utility/transform.hpp"
#include "utility/interpolate.hpp"

#include <iostream>
#include <algorithm>
#include <cmath>

namespace ts
{
  namespace world
  {   
    CarUpdateState update_car_state(const Car& car, const TerrainMap& terrain_map, double frame_duration)
    {
      using Terrain = resources::TerrainDefinition;

      CarUpdateState result;

      using Control = controls::FreeControl;
      auto throttle_rate = car.control_state(Control::Throttle) / 255.0;
      auto braking_rate = car.control_state(Control::Brake) / 255.0;
      auto turning_left_rate = car.control_state(Control::Left) / 255.0;
      auto turning_right_rate = car.control_state(Control::Right) / 255.0;
      auto turning_rate = -turning_left_rate + turning_right_rate;

      //const auto& handling = car.handling_properties();
      auto& handling_state = result.handling_state = car.handling_state();

      auto abs_turning_rate = std::abs(turning_rate);

      auto position = car.position();
      auto rotation = car.rotation();
      auto rotating_speed = car.rotating_speed();

      auto transform = make_transformation(rotation);
      auto inverse_transform = make_transformation(-rotation);

      auto velocity = car.velocity();
      auto local_velocity = transform_point(velocity, inverse_transform);

      auto local_heading = normalize(local_velocity);
      auto speed = magnitude(velocity);

      auto heading_ratio = dot_product(local_heading, make_vector2(0.0, 1.0));
      heading_ratio *= heading_ratio;

      auto slip_angle = radians(std::atan2(local_heading.x, -local_heading.y));
      if (std::abs(slip_angle.degrees()) >= 90.0)
      {
        slip_angle += degrees(180.0);
        slip_angle.normalize();
      }

      auto inv_frame_duration = 1.0 / frame_duration;

      double gravity_constant = 50.0;

      // HANDLING PROPERTIES

      struct Handling
      {
        double engine_force = 11000.0;
        double braking_force = 50000.0;

        double min_turning_radius = 35.0;
        double max_steering_angle = 10.0;
        double steering = 1.5;
        double cornering = 1.0;
        double antislide = 1.0;

        double drag_coefficient = 0.1;
        double downforce_coefficient = 0.22;
        double rolling_coefficient = 0.0;

        double traction_limit_exponent = 0.9;
        double traction_limit_factor = 4.8;

        double mass = 500.0;
        double weight_balance = 0.5;
        double wheelbase_length = 8.0;

        double max_engine_revs = 290.0;
        double reverse_gear_ratio = 2.4;

        double load_transfer_effect = 0.09;
        double load_transfer_speed = 10.0;

        std::array<double, 8> gear_ratios =
        { {
          0.0, 2.8, 2.2, 1.7, 1.3, 1.0
        } };

        std::uint32_t num_gears = 5;
        std::int32_t gear_shift_duration = 4;
      } handling{};

      auto input_moderation = 1.0;
      auto cornering_bias = 0.5;
      auto throttle_bias = 0.5;
      auto braking_bias = 0.5;
      auto antislide_bias = 0.5;
      
      // a * a + b * b + c * c + d * d = 1
      cornering_bias /= std::sqrt(cornering_bias * cornering_bias +
                                  throttle_bias * throttle_bias +
                                  braking_bias * braking_bias);
      antislide_bias /= std::sqrt(antislide_bias * antislide_bias +
                                  throttle_bias * throttle_bias +
                                  braking_bias * braking_bias);

      auto inv_total_bias = 1.0 / std::sqrt(braking_bias * braking_bias + throttle_bias * throttle_bias);
      braking_bias *= inv_total_bias;
      throttle_bias *= inv_total_bias;


      struct Axle
      {
        bool driven = false;
        double braking = 1.0;
        double downforce = 0.5;
      } front_axle, rear_axle;
      // END HANDLING PROPERTIES

      front_axle.braking = 1.0;
      rear_axle.braking = 0.5;

      //front_axle.driven = true;
      rear_axle.driven = true;

      if (braking_rate >= 0.001 && (speed < 0.001 || local_heading.y >= 0.7))
      {
        handling_state.current_gear = -1;
      }

      if (throttle_rate >= 0.001 && handling_state.current_gear < 0 && (speed < 0.001 || local_heading.y <= -0.7))
      {
        handling_state.current_gear = 0;
      }

      if (handling_state.current_gear == 0 && throttle_rate >= 0.001)
      {
        handling_state.current_gear = 1;
      }      
      
      if (handling_state.engine_rev_speed >= 0.95 && handling_state.gear_shift_state == 0 && 
          handling_state.current_gear < handling.num_gears)
      {
        handling_state.gear_shift_state = handling.gear_shift_duration;
      }

      if (handling_state.engine_rev_speed < 0.5 && handling_state.gear_shift_state == 0 &&
          handling_state.current_gear > 1)
      {
        handling_state.gear_shift_state = -handling.gear_shift_duration;
      }

      auto gear_ratio = handling.gear_ratios[handling_state.current_gear];
      if (handling_state.current_gear >= handling.gear_ratios.size())
      {
        gear_ratio = -handling.reverse_gear_ratio;
        std::swap(braking_rate, throttle_rate);
      }      

      const auto force_multiplier = frame_duration / handling.mass;      

      const auto total_downforce = speed * speed * heading_ratio * handling.downforce_coefficient;
      const auto front_downforce = total_downforce * front_axle.downforce;
      const auto rear_downforce = total_downforce * rear_axle.downforce;

      const auto front_mass = handling.mass * handling.weight_balance;
      const auto rear_mass = handling.mass - front_mass;

      const auto front_weight = front_mass * gravity_constant;
      const auto rear_weight = rear_mass * gravity_constant;

      const auto front_load = std::max(front_weight + front_downforce - handling_state.load_balance, 0.0);
      const auto rear_load = std::max(rear_weight + rear_downforce + handling_state.load_balance, 0.0);

      const auto front_traction_limit = std::pow(front_load, handling.traction_limit_exponent) * handling.traction_limit_factor;
      const auto rear_traction_limit = std::pow(rear_load, handling.traction_limit_exponent) * handling.traction_limit_factor;

      auto front_braking_force_1d = handling.braking_force * front_axle.braking * braking_rate;
      auto rear_braking_force_1d = handling.braking_force * rear_axle.braking * braking_rate;

      // 1) Determine available traction at front and rear of car.
      // 2) Adjust input parameters according to available traction
      
      auto acceleration_force_1d = handling.engine_force * std::abs(gear_ratio) * throttle_rate;
      if (handling_state.engine_rev_speed >= 1.0 || handling_state.gear_shift_state != 0)
      {
        acceleration_force_1d = 0.0;
      }

      auto front_acceleration_force_1d = 0.0;
      auto rear_acceleration_force_1d = 0.0;

      if (front_axle.driven)
      {
        front_acceleration_force_1d = acceleration_force_1d;
        if (rear_axle.driven) front_acceleration_force_1d *= 0.5;
      }

      if (rear_axle.driven)
      {
        rear_acceleration_force_1d = acceleration_force_1d;
        if (front_axle.driven) rear_acceleration_force_1d *= 0.5;
      }    
      
      auto antislide_force_1d = 0.0;
      auto front_antislide_force_1d = 0.0;
      auto cornering_force_1d = 0.0;      

      // Get the adjustment angle that corresponds to the current antislide value.
      auto turning_speed = 0.0;
      
      auto max_antislide_force = std::abs(slip_angle.radians()) * speed * rear_mass * inv_frame_duration;
      if (speed >= 0.001)
      {        
        antislide_force_1d = std::min(max_antislide_force, rear_traction_limit * handling.antislide);

        cornering_force_1d = front_traction_limit * handling.cornering * abs_turning_rate;
        turning_speed = std::min(cornering_force_1d * 0.5 / (speed * front_mass), handling.steering);

        if (handling.min_turning_radius >= 0.001)
        {
          turning_speed = std::min(turning_speed, speed / handling.min_turning_radius);
        }

        cornering_force_1d = turning_speed * speed * front_mass * 2.0;
      }

      {
        // INPUT MODERATION
        // Different bias parameters determine how much force is allocated for various things.

        auto cornering_force_orig = cornering_force_1d;
        auto antislide_force_orig = antislide_force_1d;

        antislide_force_1d = std::min(antislide_bias * rear_traction_limit, antislide_force_1d);

        auto remaining_antislide = max_antislide_force - antislide_force_1d;
        cornering_force_1d = std::min(cornering_bias * front_traction_limit, std::max(remaining_antislide, cornering_force_1d));

        auto moderate_input = [&](double avail_sq,
                                  double& accel_force, double& braking_force,
                                  double& accel_force_other, double& braking_force_other)
        {
          auto braking_sq = braking_force * braking_force;
          auto accel_sq = accel_force * accel_force;

          avail_sq = std::max(avail_sq, 0.0);
          if (braking_sq + accel_sq >= avail_sq)
          {
            auto available_force = std::sqrt(avail_sq);

            auto braking_threshold = braking_bias * available_force;
            auto acceleration_threshold = throttle_bias * available_force;

            if (braking_force < braking_threshold && accel_force >= 0.01)
            {
              avail_sq -= braking_sq;
              if (avail_sq < accel_sq)
              {
                auto force = std::sqrt(avail_sq);
                auto factor = force / accel_force;

                accel_force = force;
                accel_force_other *= factor;
              }
            }

            else if (accel_force < acceleration_threshold && braking_force >= 0.01)
            {
              avail_sq -= accel_sq;
              if (avail_sq < braking_sq)
              {
                auto force = std::sqrt(avail_sq);
                auto factor = force / braking_force;

                braking_force = force;
                braking_force_other *= factor;
              }
            }

            else 
            {
              if (braking_force >= 0.01)
              {
                auto braking_factor = braking_threshold / braking_force;
                braking_force = braking_threshold;
                braking_force_other *= braking_factor;
              }
              
              if (accel_force >= 0.01)
              {
                auto accel_factor = acceleration_threshold / accel_force;
                accel_force = acceleration_threshold;
                accel_force_other *= accel_factor;
              }         
            }
          }
        };

        moderate_input(front_traction_limit * front_traction_limit - cornering_force_1d * cornering_force_1d,
                       front_acceleration_force_1d, front_braking_force_1d,
                       rear_acceleration_force_1d, rear_braking_force_1d);

        moderate_input(rear_traction_limit * rear_traction_limit - antislide_force_1d * antislide_force_1d,
                       rear_acceleration_force_1d, rear_braking_force_1d,
                       front_acceleration_force_1d, front_braking_force_1d);

        auto available_cornering_force_sq = front_traction_limit * front_traction_limit -
          front_acceleration_force_1d * front_acceleration_force_1d - front_braking_force_1d * front_braking_force_1d;

        cornering_force_1d = std::min(std::sqrt(available_cornering_force_sq), cornering_force_orig);

        if (cornering_force_1d < cornering_force_orig)
        {
          abs_turning_rate *= cornering_force_1d / cornering_force_orig;
        }

        // Front antislide
        front_antislide_force_1d = std::max(cornering_force_1d - cornering_force_orig, 0.0);

        auto available_antislide_force_sq = rear_traction_limit * rear_traction_limit -
          rear_acceleration_force_1d * rear_acceleration_force_1d - rear_braking_force_1d * rear_braking_force_1d;

        antislide_force_1d = std::min(std::sqrt(available_antislide_force_sq), antislide_force_orig);
      }

      
      auto turning_correction = make_vector2(0.0, 0.0);      

      if (speed >= 0.001 && std::abs(cornering_force_1d) >= 0.001)
      {
        turning_speed = std::abs(cornering_force_1d * 0.5) / (speed * front_mass);
        if (turning_rate < 0.0) turning_speed = -turning_speed;

        auto half_wheelbase = make_vector2(0.0, -16 * 0.5);
        turning_correction = (transform_point(half_wheelbase, radians(turning_speed * frame_duration)) - half_wheelbase) *
          inv_frame_duration * handling.mass;

        //turning_speed = interpolate_linearly(handling.steering, turning_speed, std::min(antislide_speed / turning_speed, 1.0)); 
      }

      auto antislide_force = make_vector2(0.0, 0.0);
      if (speed >= 0.001)
      {       
        auto antislide_speed = antislide_force_1d / (rear_mass * speed) + front_antislide_force_1d / (front_mass * speed);
        antislide_force = transform_point(make_vector2(antislide_force_1d + front_antislide_force_1d, 0.0), 
                                          radians(antislide_speed * frame_duration));
        if (dot_product(local_heading, antislide_force) >= 0.0)
        {
          antislide_force = -antislide_force;
        }

        antislide_force = {};
      }      

      if (gear_ratio < 0)
      {
        front_acceleration_force_1d = -front_acceleration_force_1d;
        rear_acceleration_force_1d = -rear_acceleration_force_1d;
      }     

      auto steering_angle = handling.max_steering_angle * abs_turning_rate;
      if (turning_rate < 0.0) steering_angle = -steering_angle;

      auto front_wheel_vector = transform_point(make_vector2(0.0, -1.0), degrees(steering_angle));
      
      auto net_force = rear_acceleration_force_1d * make_vector2(0.0, -1.0) +
        front_acceleration_force_1d * front_wheel_vector +
        (front_braking_force_1d + rear_braking_force_1d) * -local_heading +
        antislide_force + turning_correction;

      auto new_velocity = local_velocity + net_force * force_multiplier;      

      // TODO: Traction limit/input moderation
      // TODO: Rolling resistance
      // TODO: Slide friction
      // TODO: Load/weight balance stuff
      // TODO: Rotational speed fix

      auto drag_resistance = new_velocity * magnitude(new_velocity) * handling.drag_coefficient;
      net_force -= drag_resistance;
      new_velocity -= drag_resistance * force_multiplier;      

      if (dot_product(local_velocity, new_velocity) < 0.0)
      {
        new_velocity = {};
      }

      if (handling_state.gear_shift_state > 0)
      {
        auto next = handling_state.current_gear + 1;
        auto target_revs = (magnitude(new_velocity) / handling.max_engine_revs) * handling.gear_ratios[next];

        handling_state.engine_rev_speed += (target_revs - handling_state.engine_rev_speed) / handling_state.gear_shift_state;
        handling_state.gear_shift_state--;

        if (handling_state.gear_shift_state == 0)
        {
          handling_state.current_gear = next;
        }
      }

      else if (handling_state.gear_shift_state < 0)
      {
        auto prev = handling_state.current_gear - 1;
        auto target_revs = (magnitude(new_velocity) / handling.max_engine_revs) * handling.gear_ratios[prev];

        handling_state.engine_rev_speed += (target_revs - handling_state.engine_rev_speed) / -handling_state.gear_shift_state;
        handling_state.gear_shift_state++;

        if (handling_state.gear_shift_state == 0)
        {
          handling_state.current_gear = prev;
        }
      }

      else
      {
        handling_state.engine_rev_speed = magnitude(new_velocity) / handling.max_engine_revs * std::abs(gear_ratio);
      }

      {
        printf("X: %f (%f %f)\n", net_force.x / (handling.mass * 9.8 * 3.6), turning_correction.x, turning_correction.y);
        printf("Y: %f\n", net_force.y / (handling.mass * 9.8 * 3.6));

        handling_state.load_balance = interpolate_linearly(handling_state.load_balance,
                                                           handling.load_transfer_effect * -net_force.y,
                                                           std::min(handling.load_transfer_speed * frame_duration, 1.0));
      }

      result.rotating_speed = turning_speed;
      result.velocity = transform_point(new_velocity, transform);

      return result;
    }
  }
}


  /*

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

  auto torque_multiplier = throttle_rate;
  if (old_gear != handling_state.current_gear || handling_state.engine_rev_speed > 1.0)
  {
  //torque_multiplier = 0.0;
  }

  torque_multiplier *= gear_ratio;

  struct WheelState
  {
  double mass;
  double load;

  Vector2d acceleration;
  Vector2d position;

  Vector2d net_force;
  Vector2d rotational_impulse;
  double rotational_acceleration;
  };

  boost::container::small_vector<WheelState, 4> wheel_states;
  auto mass = static_cast<double>(handling.mass);

  auto axles = handling.axles;
  {
  auto& front = axles[0];
  auto& rear = axles[1];

  front.antislide = 0.0;
  rear.antislide = 1.0;
  front.driven = false;
  rear.driven = true;
  front.braking = 1.0;
  rear.braking = 0.8;
  front.cornering = 1.0;
  rear.cornering = 0.0;
  front.downforce = 1.0;
  rear.downforce = 1.0;

  front.wheels = { make_vector2(-3.0, -5.0), make_vector2(3.0, -5.0) };
  rear.wheels = { make_vector2(-3.0, 5.0), make_vector2(3.0, 5.0) };
  }


  // Calculate drag resistance for downforce. The actual drag effect will be calculated
  // based on the car's new velocity.
  auto drag_resistance = local_velocity * speed * handling.drag_coefficient;
  auto total_downforce = magnitude(drag_resistance) * oversteer_ratio * handling.downforce_coefficient;

  double total_mass = 0.0;
  for (auto& axle : axles)
  {
  for (auto wheel_pos : axle.wheels)
  {
  auto wheel_offset = wheel_pos - handling.center_of_mass;

  WheelState ws{};
  ws.mass = 1.0 / std::max(magnitude(wheel_offset), 1.0);
  ws.position = wheel_pos;

  ws.acceleration = handling_state.acceleration +
  make_vector2(-wheel_pos.y, wheel_pos.x) * handling_state.rotational_acceleration;

  ws.load = dot_product(ws.acceleration, wheel_offset * handling.load_transfer_coefficient) +
  total_downforce * axle.downforce;

  total_mass += ws.mass;
  wheel_states.push_back(ws);
  }
  }

  if (total_mass >= 0.001)
  {
  auto mass_multiplier = handling.mass / total_mass;

  for (auto& wheel : wheel_states)
  {
  wheel.mass *= mass_multiplier;
  wheel.load += wheel.mass * 100.0; // gravity constant

  if (wheel.load < 0.0) wheel.load = 0.0;
  }
  }


  auto driven_axles = std::count_if(axles.begin(), axles.end(),
  [](const auto& axle)
  {
  return axle.driven;
  });

  auto inv_peak_slip_angle = 1.0 / std::max(handling.peak_slip_angle, 1.0);

  auto wheel_it = wheel_states.begin();
  for (const auto& axle : axles)
  {
  auto acceleration_force_1d = 0.0;

  if (axle.driven)
  {
  // Apply torque to the axle's wheels.
  acceleration_force_1d += handling.torque * torque_multiplier / (driven_axles * axle.wheels.size());
  }

  auto braking_force_1d = axle.braking * braking_rate * handling.braking;

  auto steering_angle = degrees(handling.max_steering_angle * turning_rate * axle.cornering);
  auto wheel_direction = transform_point(make_vector2(0.0, -1.0), steering_angle);

  for (auto wheel_pos : axle.wheels)
  {
  auto traction_limit = std::pow(wheel_it->load, 0.95) * 2.5;

  auto wheel_velocity = local_velocity + rotating_speed * make_vector2(-wheel_pos.y, wheel_pos.x);
  auto wheel_heading = normalize(wheel_velocity);

  auto wheel_angle = radians(std::atan2(wheel_direction.x, -wheel_direction.y));
  auto wheel_heading_angle = radians(std::atan2(wheel_heading.x, -wheel_heading.y));

  auto slip_angle = normalize(wheel_heading_angle - wheel_angle);
  auto reverse_slip_angle = normalize(degrees(180.0) + slip_angle);

  auto cornering_force_direction = make_vector2(-1.0, 0.0);
  if (slip_angle.radians() < 0.0) cornering_force_direction.x = -cornering_force_direction.x;

  auto effective_wheel_direction = wheel_direction;
  if (std::abs(reverse_slip_angle.radians()) < std::abs(slip_angle.radians()))
  {
  std::swap(slip_angle, reverse_slip_angle);
  effective_wheel_direction = -effective_wheel_direction;
  cornering_force_direction = -cornering_force_direction;
  }

  // Calculate the cornering force.
  auto slip_degrees = slip_angle.degrees();

  auto slip_ratio = std::min(std::abs(slip_degrees) * inv_peak_slip_angle, 1.0);
  auto grip_ratio = slip_ratio;
  if (slip_ratio > 0.5)
  {
  grip_ratio = 1.0 - slip_ratio;
  slip_ratio = ((slip_ratio - 0.5) * 2.0);
  }

  else
  {
  slip_ratio = 0.0;
  }

  auto wheel_speed = std::abs(dot_product(effective_wheel_direction, wheel_velocity));

  auto slide_friction_direction = normalize(effective_wheel_direction * wheel_speed - wheel_velocity);
  auto slide_friction = slide_friction_direction * handling.slide_friction * wheel_it->load * slip_ratio;

  //auto max_cornering_force_1d = wheel_it->mass * std::abs(dot_product(cornering_force_direction, wheel_velocity));
  auto cornering_force_1d = grip_ratio * wheel_it->load;
  auto cornering_force = cornering_force_direction * cornering_force_1d;

  wheel_it->net_force += (wheel_direction * acceleration_force_1d);
  wheel_it->net_force -= wheel_heading * braking_force_1d;

  wheel_it->net_force += cornering_force;
  wheel_it->net_force += slide_friction;

  auto rolling_resistance = wheel_heading * wheel_it->load * handling.rolling_resistance_factor;
  wheel_it->net_force -= rolling_resistance;

  ++wheel_it;
  }
  }

  for (WheelState& wheel : wheel_states)
  {
  auto offset = wheel.position - handling.center_of_mass;

  auto pivot = make_vector2(-offset.y, offset.x);
  auto arm_length = magnitude(pivot);
  if (arm_length >= 0.001)
  {
  pivot /= arm_length;

  auto force_direction = normalize(wheel.net_force);
  auto force_magnitude = magnitude(wheel.net_force);

  auto impulse = dot_product(pivot, force_direction);

  wheel.rotational_impulse = wheel.net_force * std::abs(impulse);
  wheel.rotational_acceleration = (impulse * force_magnitude) / (handling.mass * arm_length);
  }
  }


  auto net_rotational_acceleration = 0.0;
  {
  auto partition_it = std::partition(wheel_states.begin(), wheel_states.end(),
  [](const WheelState& ws)
  {
  return ws.rotational_acceleration < 0.0;
  });

  auto accum = [](double a, const WheelState& b)
  {
  return a + b.rotational_acceleration;
  };

  auto total_neg = std::accumulate(wheel_states.begin(), partition_it, 0.0, accum);
  auto total_pos = std::accumulate(partition_it, wheel_states.end(), 0.0, accum);

  auto apply_rotational_acceleration = [&](WheelState ws, double ratio)
  {
  ws.net_force -= ws.rotational_impulse * ratio;
  return ws;
  };

  net_rotational_acceleration = total_neg + total_pos;
  if (net_rotational_acceleration < 0.0)
  {
  auto apply = std::bind(apply_rotational_acceleration, std::placeholders::_1, net_rotational_acceleration / total_neg);
  std::transform(wheel_states.begin(), partition_it, wheel_states.begin(), apply);
  }

  else if (net_rotational_acceleration > 0.0)
  {
  auto apply = std::bind(apply_rotational_acceleration, std::placeholders::_1, net_rotational_acceleration / total_pos);
  std::transform(partition_it, wheel_states.end(), partition_it, apply);
  }
  }

  Vector2d net_force{};
  for (const WheelState& wheel : wheel_states)
  {
  net_force += wheel.net_force;
  }

  auto inv_force_multiplier = handling.mass / frame_duration;
  auto force_multiplier = frame_duration / handling.mass;

  auto new_local_velocity = local_velocity + net_force * force_multiplier;
  drag_resistance = new_local_velocity * magnitude(new_local_velocity) * handling.drag_coefficient;

  new_local_velocity -= drag_resistance * force_multiplier;

  handling_state.acceleration = (new_local_velocity - local_velocity) / frame_duration;
  handling_state.rotational_acceleration = net_rotational_acceleration;

  result.rotating_speed = rotating_speed + net_rotational_acceleration * frame_duration;
  result.velocity = transform_point(new_local_velocity, transform);

  /*
  auto movement_angle = radians(std::atan2(velocity.x, -velocity.y));
  auto slip_angle = rotation - movement_angle;
  if (dot_product(velocity, facing_vector) < 0.0)
  {
  slip_angle -= degrees(180.0);
  }
  slip_angle.normalize();

  auto slip_radians = std::abs(slip_angle.radians());
  */

  /*
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

  */

  // Apply the acceleration force
  /*
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
 */