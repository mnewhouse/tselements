/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "handling_v2.hpp"
#include "car.hpp"

#include "utility/transform.hpp"
#include "utility/interpolate.hpp"

namespace ts
{
  namespace world
  {
    struct Handling
    {
      double acceleration_force = 15000.0;
      double max_engine_revs = 300.0;

      double base_braking = 10000.0;

      double traction_limit_exponent = 1.0;
      double traction_limit_factor = 2.0;
      
      double drag_coefficient = 0.5;
      double downforce_coefficient = 0.0;

      double max_steering_angle = 15.0;
      double non_slide_angle = 5.0;
      double full_slide_angle = 10.0;
      double slide_friction = 0.7;
      double cornering = 1.0;

      double length = 16.0;

      struct Axle
      {
        bool driven = false;

        double steering = 0.0;
        double braking = 1.0;
        double downforce = 0.5;

        std::size_t num_wheels = 2;
        std::array<Vector2d, 2> wheel_positions;
      };      

      std::array<Axle, 2> axles;

      std::size_t num_gears = 5;
      std::array<double, 8> gear_ratios = { 2.8, 2.184, 1.70352, 1.32875, 1.03642 };
      double reverse_gear_ratio = 3.0;

      int gear_shift_duration = 3;
    };

    HandlingState apply_physics_forces(Car& car, const TerrainMap& terrain_map,
                                       double frame_duration)
    {
      Handling handling{};
      auto& front_axle = handling.axles[0];
      auto& rear_axle = handling.axles[1];

      front_axle.driven = false;
      rear_axle.driven = true;

      front_axle.steering = 1.0;
      rear_axle.steering = 0.0;

      front_axle.wheel_positions = { { { -4.0, -6.0 }, { 4.0, -6.0 } } };
      rear_axle.wheel_positions = { { { -4.0, 6.0 },{ 4.0, 6.0 } } };

      const auto gravity_constant = 50.0;

      using Control = controls::FreeControl;
      auto throttle_rate = car.control_state(Control::Throttle) / 255.0;
      auto braking_rate = car.control_state(Control::Brake) / 255.0;
      auto turning_left_rate = car.control_state(Control::Left) / 255.0;
      auto turning_right_rate = car.control_state(Control::Right) / 255.0;
      auto turning_rate = -turning_left_rate + turning_right_rate;
      auto abs_turning_rate = std::abs(turning_rate);

      //const auto& handling = car.handling_properties();
      auto handling_state = car.handling_state();

      auto position = car.position();
      auto rotation = car.rotation();
      auto angular_velocity = car.angular_velocity();

      auto transform = make_transformation(rotation);
      auto inverse_transform = make_transformation(-rotation);

      auto velocity = car.velocity();
      auto local_velocity = transform_point(velocity, inverse_transform);

      auto local_heading = normalize(local_velocity);
      auto speed = magnitude(velocity);

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

      auto acceleration_force_1d = handling.acceleration_force * std::abs(gear_ratio) * throttle_rate;
      if (handling_state.engine_rev_speed >= 1.0 || handling_state.gear_shift_state != 0)
      {
        acceleration_force_1d = 0.0;
      }

      auto braking_force_1d = handling.base_braking * braking_rate;

      auto axles_driven = front_axle.driven + rear_axle.driven;
      auto drive_ratio = axles_driven ? 1.0 / axles_driven : 0.0;

      auto center_of_mass = car.center_of_mass();
      auto mass = car.mass();

      struct AxleState
      {
        std::array<double, 2> wheel_load;
      };

      std::array<AxleState, 2> axle_states{};

      auto axle_state = axle_states.begin();

      double total_load = 0.0;
      for (auto& axle : handling.axles)
      {
        for (unsigned i = 0; i < axle.num_wheels; ++i)
        {
          auto wheel = axle.wheel_positions[i];
          auto load_value = gravity_constant / std::max(magnitude(wheel - center_of_mass), 1.0);

          axle_state->wheel_load[i] = load_value;          

          total_load += load_value;
        }

        ++axle_state;
      }

      if (total_load >= 0.001)
      {
        auto load_multiplier = mass * gravity_constant / total_load;
        for (auto& as : axle_states)
        {
          for (auto& load : as.wheel_load)
          {
            load *= load_multiplier;
          }
        }
      }

      const auto total_downforce = local_velocity.y * local_velocity.y * handling.downforce_coefficient;
      const auto front_downforce = total_downforce * front_axle.downforce * 0.5;
      const auto rear_downforce = total_downforce * rear_axle.downforce * 0.5;

      axle_states[0].wheel_load[0] += front_downforce;
      axle_states[0].wheel_load[1] += front_downforce;
      axle_states[1].wheel_load[0] += rear_downforce;
      axle_states[1].wheel_load[1] += rear_downforce;      

      auto apply_axle_forces = [&](const Handling::Axle& axle, AxleState& axle_state)
      {
        if (axle.num_wheels != 0)
        {
          auto steering_angle = degrees(turning_rate * axle.steering * handling.max_steering_angle);          
          auto wheel_facing = transform_point(make_vector2(0.0, -1.0), steering_angle);

          auto acceleration = acceleration_force_1d * (axle.driven * drive_ratio) / axle.num_wheels;
          auto braking = braking_force_1d * axle.braking;

          for (std::uint32_t i = 0; i < axle.num_wheels; ++i)
          {
            auto load = axle_state.wheel_load[i];
            auto traction_limit = std::pow(load, handling.traction_limit_exponent) * handling.traction_limit_factor;            

            auto wheel_pos = axle.wheel_positions[i];            

            auto wheel_velocity = make_vector2(-wheel_pos.y, wheel_pos.x) * car.angular_velocity();
            wheel_velocity += local_velocity;

            auto cornering_force_direction = make_vector2(-wheel_facing.y, wheel_facing.x);
            if ((cornering_force_direction.x < 0.0) != (turning_rate < 0.0))
            {
              cornering_force_direction = -cornering_force_direction;
            }

            auto max_cornering_force = std::abs(cross_product(mass * wheel_velocity, wheel_facing));
            auto cornering_force = std::min(traction_limit * abs_turning_rate * axle.steering,
                                            max_cornering_force) * cornering_force_direction;

            auto braking_force = -normalize(wheel_velocity) * braking;

            car.apply_force(wheel_facing * acceleration, wheel_pos);            
            car.apply_force(cornering_force, wheel_pos);
            car.apply_force(braking_force, wheel_pos);
          }
        }
      };

      apply_axle_forces(front_axle, axle_states[0]);
      apply_axle_forces(rear_axle, axle_states[1]);

      auto drag_resistance = local_velocity * speed * handling.drag_coefficient;
      
      car.apply_force({}, drag_resistance);

      return handling_state;
    }
  }
}