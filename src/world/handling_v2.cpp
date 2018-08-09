/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "handling_v2.hpp"
#include "car.hpp"

#include "utility/transform.hpp"
#include "utility/interpolate.hpp"
#include "utility/math_utilities.hpp"

#include <boost/container/small_vector.hpp>

#include <array>
#include <algorithm>

namespace ts
{
  namespace world
  {    
    struct Handling
    {
      double max_acceleration_force = 12000.0;
      double max_engine_revs = 300.0;      

      boost::container::small_vector<double, 8> gear_ratios = { 2.5, 2.0, 1.6, 1.3, 1.1 };
      double reverse_gear_ratio = 2.5;
      int gear_shift_duration = 3;

      double max_braking_force = 30000.0;
      double drag_coefficient = 0.14;
      double downforce_coefficient = 0.18;
      double rolling_drag_coefficient = 0.03;

      double traction_limit = 50000.0;
      double load_transfer = 0.2;

      double brake_balance = 0.4;
      double downforce_balance = 0.52;
      double steering_balance = 0.0;
      
      double cornering = 1.0;      

      double min_turning_radius = 25.0;
      double non_slide_angle = 5.0;
      double full_slide_angle = 10.0;
      double sliding_grip = 0.7;

      double wheelbase_length = 18.0;
      double wheelbase_offset = 0.0;
      double num_front_wheels = 2;
      double num_rear_wheels = 2;
      double front_axle_width = 8.0;
      double rear_axle_width = 8.0;
      
      bool front_driven = false;
      bool rear_driven = true;      
    };

    HandlingState apply_physics_forces(Car& car, const TerrainMap& terrain_map,
                                     double frame_duration)
    {
      Handling handling{};

      using controls::Control;
      auto half_wheelbase = handling.wheelbase_length * 0.5;

      std::array<Vector2d, 2> front_wheel_positions =
      { {
        { 0.0, -half_wheelbase + handling.wheelbase_offset }
        } };

      std::array<Vector2d, 2> rear_wheel_positions =
      { {
        { 0.0, half_wheelbase + handling.wheelbase_offset }
        } };

      if (handling.num_front_wheels >= 2)
      {
        front_wheel_positions =
        { {
          { -handling.front_axle_width * 0.5, -half_wheelbase + handling.wheelbase_offset },
          { handling.front_axle_width * 0.5, -half_wheelbase + handling.wheelbase_offset },
        } };
      }

      if (handling.num_rear_wheels >= 2)
      {
        rear_wheel_positions =
        { {
          { -handling.rear_axle_width * 0.5, half_wheelbase + handling.wheelbase_offset },
          { handling.rear_axle_width * 0.5, half_wheelbase + handling.wheelbase_offset }
        } };
      }

      auto transform = make_transformation(car.rotation());
      auto inv_transform = make_transformation(-car.rotation());

      auto handling_state = car.handling_state();

      auto local_velocity = transform_point(car.velocity(), inv_transform);
      auto local_heading = normalize(local_velocity);
      auto speed = magnitude(local_velocity);

      auto num_wheels = handling.num_front_wheels + handling.num_rear_wheels;
      auto inv_num_wheels = 1.0 / num_wheels;
      auto wheel_traction_limit = handling.traction_limit * inv_num_wheels;

      auto total_downforce = local_heading.y * local_heading.y * speed * speed * handling.downforce_coefficient;
      if (local_heading.y > 0.0) total_downforce = 0.0;

      auto center_of_mass = car.center_of_mass();
      auto weight_distribution = clamp(center_of_mass.y / (handling.wheelbase_length * 0.5) + 0.5, 0.5, 0.5);

      auto rear_downforce = total_downforce * handling.downforce_balance;
      auto front_downforce = total_downforce - rear_downforce;

      auto inv_num_front_wheels = 1.0 / handling.num_front_wheels;
      auto inv_num_rear_wheels = 1.0 / handling.num_rear_wheels;     

      auto front_base_load = wheel_traction_limit * 2.0 * (1.0 - weight_distribution);
      auto rear_base_load = wheel_traction_limit * 2.0 * weight_distribution;

      auto front_load_transfer = -handling_state.load_transfer * inv_num_wheels;
      auto rear_load_transfer = handling_state.load_transfer * inv_num_wheels;

      auto front_traction_limit = std::max(front_base_load + front_load_transfer + front_downforce * inv_num_front_wheels, 0.0);
      auto rear_traction_limit = std::max(rear_base_load + rear_load_transfer + rear_downforce * inv_num_front_wheels, 0.0);

      auto throttle_rate = car.control_state(Control::Throttle) / 255.0;
      auto braking_rate = car.control_state(Control::Brake) / 255.0;
      auto turning_left_rate = car.control_state(Control::Left) / 255.0;
      auto turning_right_rate = car.control_state(Control::Right) / 255.0;
      auto turning_rate = -turning_left_rate + turning_right_rate;
      auto net_throttle = throttle_rate - braking_rate;

      auto front_acceleration = 0.0;
      auto rear_acceleration = 0.0;
      if (handling.front_driven && handling.rear_driven)
      {
        front_acceleration = 0.5 * inv_num_front_wheels;
        rear_acceleration = 0.5 * inv_num_rear_wheels;
      }

      else if (handling.front_driven)
      {
        front_acceleration = inv_num_front_wheels;
      }

      else if (handling.rear_driven)
      {
        rear_acceleration = inv_num_rear_wheels;
      }

      if ((speed <= 0.0001 || local_heading.y > 0.7) && net_throttle < 0.0)
      {
        handling_state.current_gear = -1;
      }

      if ((speed <= 0.0001 || local_heading.y > -0.7) && net_throttle > 0.0)
      {
        handling_state.current_gear = 1;
      }

      auto num_gears = static_cast<int>(handling.gear_ratios.size());     
      if (handling_state.gear_shift_state != 0)
      {
        auto new_gear = handling_state.current_gear;
        if (handling_state.gear_shift_state > 0) ++new_gear;
        else --new_gear;

        auto old_ratio = handling.gear_ratios[handling_state.current_gear - 1];
        auto new_ratio = handling.gear_ratios[new_gear - 1];

        auto frame_progress = 1.0 / handling.gear_shift_duration;
        auto progress = 1.0 - (std::abs(handling_state.gear_shift_state) * frame_progress);

        auto current_ratio = interpolate_linearly(old_ratio, new_ratio, progress);
        auto updated_ratio = interpolate_linearly(old_ratio, new_ratio, progress + frame_progress);
        
        handling_state.engine_rev_speed /= current_ratio;
        handling_state.engine_rev_speed *= updated_ratio;

        if (handling_state.gear_shift_state > 0) --handling_state.gear_shift_state;
        else ++handling_state.gear_shift_state;

        if (handling_state.gear_shift_state == 0)
        {
          handling_state.current_gear = new_gear;
        }
      }

      else if (handling_state.engine_rev_speed >= 0.95)
      {
        if (handling_state.current_gear >= 1 && handling_state.current_gear < num_gears)
        {
          handling_state.gear_shift_state = handling.gear_shift_duration;          
        }
      }

      else if (handling_state.engine_rev_speed < 0.8 && handling_state.current_gear > 1)
      {
        auto prev_gear = handling_state.current_gear - 1;
        auto new_revs = handling_state.engine_rev_speed;
        new_revs /= handling.gear_ratios[handling_state.current_gear - 1];
        new_revs *= handling.gear_ratios[prev_gear - 1];
        if (new_revs < 0.8)
        {
          handling_state.gear_shift_state = -handling.gear_shift_duration;
        }          
      }

      auto gear_ratio = 1.0;
      if (handling_state.current_gear < 0)
      {
        net_throttle = -net_throttle;
        gear_ratio = -handling.reverse_gear_ratio;
      }

      else if (handling_state.current_gear == 0)
      {
        gear_ratio = 0.0;
      }

      else if (handling_state.current_gear - 1 < num_gears)
      {
        gear_ratio = handling.gear_ratios[handling_state.current_gear - 1];
      }

      if (handling_state.gear_shift_state == 0)
      {
        handling_state.engine_rev_speed = std::min(speed * std::abs(gear_ratio) / handling.max_engine_revs, 1.05);        
      }

      else
      {
        gear_ratio = 0.0;
      }
      
      auto throttle_factor = 1.0;
      if (handling_state.engine_rev_speed > 1.0)
      {
        throttle_factor = 0.0;
      }
      
      auto acceleration_force_1d = std::max(net_throttle, 0.0) * handling.max_acceleration_force * gear_ratio * throttle_factor;
      auto braking_force_1d = std::max(-net_throttle, 0.0) * handling.max_braking_force;

      struct WheelState
      {
        Vector2d pos;
        Vector2d velocity;
        Vector2d bias;
        Vector2d wheel_facing;
        Vector2d provisional_force;
        double heading_angle;
        double traction_limit;
        double acceleration;
        double braking;        
        double cornering;
        double max_steering_angle;
        double acceleration_force;
        double braking_force;
        double antislide_force;       
        double slide_ratio;
      };

      auto throttle_bias = 1.0;
      auto brake_bias = 1.0;
      auto cornering_bias = 1.0;
      auto antislide_bias = 1.0;
      auto longitudinal_bias = net_throttle >= 0.0 ? throttle_bias : brake_bias;

      auto cornering_bias_2d = normalize(make_vector2(cornering_bias, longitudinal_bias));
      auto antislide_bias_2d = normalize(make_vector2(antislide_bias, longitudinal_bias));

      boost::container::small_vector<WheelState, 4> wheel_states;
      for (auto p : front_wheel_positions)
      {
        WheelState ws{};
        ws.pos = p;
        ws.traction_limit = front_traction_limit;
        ws.acceleration = front_acceleration;
        ws.braking = 1.0 - handling.brake_balance;
        ws.cornering = handling.cornering;
        ws.bias = cornering_bias_2d;
        wheel_states.push_back(ws);
      }

      for (auto p : rear_wheel_positions)
      {
        WheelState ws{};
        ws.pos = p;
        ws.traction_limit = rear_traction_limit;
        ws.acceleration = rear_acceleration;
        ws.braking = handling.brake_balance;        
        ws.cornering = handling.cornering;
        ws.bias = antislide_bias_2d;
        wheel_states.push_back(ws);
      }

      auto momentum_1d = speed * car.mass();
      auto is_turning = std::abs(turning_rate) >= 0.0001;

      auto turning_circle_center = make_vector2(0.0, 0.0);
      turning_circle_center.y = handling.wheelbase_length * (0.5 - handling.steering_balance);
      turning_circle_center.x = handling.min_turning_radius;

      if (turning_rate < 0.0)
      {
        turning_circle_center.x = -turning_circle_center.x;
        turning_rate = -turning_rate;
      }

      auto adjusted_steering_rate = 0.0;
      for (auto& ws : wheel_states)
      {
        ws.velocity = local_velocity + car.angular_velocity() * make_vector2(-ws.pos.y, ws.pos.x);
        ws.heading_angle = std::atan2(ws.velocity.x, -ws.velocity.y);

        auto facing_limit = tangent(ws.pos - turning_circle_center);
        if (facing_limit.y > 0.0) facing_limit = -facing_limit;        

        ws.max_steering_angle = std::atan2(facing_limit.x, -facing_limit.y);
        if (is_turning && std::abs(ws.max_steering_angle) >= 0.00001)
        {
          auto desired_angle = degrees(handling.non_slide_angle);
          if (ws.max_steering_angle < 0.0) desired_angle = -desired_angle;        

          auto steering_angle = radians(ws.heading_angle) + desired_angle;
          auto steering_rate = std::min(steering_angle.radians() / ws.max_steering_angle, 1.0);

          if (steering_rate > adjusted_steering_rate)
          {
            adjusted_steering_rate = steering_rate;
          }
        }
      }

      for (auto& ws : wheel_states)
      {
        ws.wheel_facing = make_vector2(0.0, -1.0);
        auto steering_angle = radians(0.0);
        if (is_turning)
        {
          steering_angle = radians(adjusted_steering_rate * ws.max_steering_angle);
          ws.wheel_facing = transform_point(ws.wheel_facing, steering_angle);          
        }

        auto slip_angle = radians(ws.heading_angle) - steering_angle;
        if (std::abs(slip_angle.degrees()) >= 90.0)
        {
          slip_angle -= degrees(180.0);
          slip_angle.normalize();
        }

        auto slip_degrees = std::abs(slip_angle.degrees());
        
        ws.slide_ratio = 1.0;
        auto antislide_ratio = 1.0;
        if (slip_degrees < handling.full_slide_angle)
        {
          antislide_ratio = slip_degrees / handling.full_slide_angle;

          if (slip_degrees < handling.non_slide_angle)
          {
            ws.slide_ratio = 0.0;            
          }

          else
          {
            ws.slide_ratio = (slip_degrees / handling.non_slide_angle) /
              (handling.full_slide_angle - handling.non_slide_angle);
          }
        }        
        
        ws.traction_limit *= interpolate_linearly(1.0, handling.sliding_grip, ws.slide_ratio);

        auto max_antislide = std::abs(dot_product(ws.velocity * car.mass() * inv_num_wheels, ws.wheel_facing));

        ws.antislide_force = std::min(antislide_ratio * ws.cornering * ws.traction_limit, max_antislide);
        ws.acceleration_force = ws.acceleration * acceleration_force_1d;
        ws.braking_force = ws.braking * braking_force_1d;
      }
      
      auto pedal_adjustment = 1.0;
      for (auto& ws : wheel_states)
      {
        auto longitudinal_force = (std::abs(ws.acceleration_force) + std::abs(ws.braking_force)) * pedal_adjustment;
        auto applied_force = make_vector2(ws.antislide_force, longitudinal_force);
      
        if (magnitude_squared(applied_force) > ws.traction_limit * ws.traction_limit)
        {
          auto reduce_x = ws.antislide_force > ws.bias.x * ws.traction_limit;
          auto reduce_y = longitudinal_force > ws.bias.y * ws.traction_limit;

          if (reduce_y)
          {
            if (reduce_x)
            {
              pedal_adjustment = (ws.traction_limit * ws.bias.y) / (std::abs(ws.acceleration_force) + std::abs(ws.braking_force));
            }

            else
            {
              auto rem = ws.traction_limit * ws.traction_limit - ws.antislide_force * ws.antislide_force;
              pedal_adjustment = std::sqrt(rem) / (std::abs(ws.acceleration_force) + std::abs(ws.braking_force));
            }
          }
        }
      }

      for (auto& ws : wheel_states)
      {        
        auto longitudinal_force = (std::abs(ws.acceleration_force) + ws.braking_force) * pedal_adjustment;
        auto rem = std::max(ws.traction_limit * ws.traction_limit - longitudinal_force * longitudinal_force, 0.0);

        if (rem < ws.antislide_force * ws.antislide_force)
        {
          ws.antislide_force = std::sqrt(rem);
        }
      }

      auto net_force = make_vector2(0.0, 0.0);
      for (auto& ws : wheel_states)
      {
        auto force =  make_vector2(0.0, 0.0);

        auto wheel_heading = normalize(ws.velocity);
        auto target_heading = ws.wheel_facing;
        if (dot_product(wheel_heading, target_heading) < 0.0) target_heading = -target_heading;

        force += ws.acceleration_force * pedal_adjustment * ws.wheel_facing;          
        force += ws.braking_force * pedal_adjustment * -wheel_heading;

        auto antislide_direction = normalize(target_heading - wheel_heading);
        force += ws.antislide_force * antislide_direction;

        auto rolling_resistance = (1.0 - ws.slide_ratio) * handling.rolling_drag_coefficient * ws.traction_limit;
        force += rolling_resistance * -wheel_heading;

        car.apply_force(force, ws.pos);        

        net_force += force;        
      }

      auto drag = speed * -local_velocity * handling.drag_coefficient;
      car.apply_force(drag, {});      

      net_force += drag;

      for (auto& ws : wheel_states)
      {
        auto pos = ws.pos - car.center_of_mass();
        auto direction = make_vector2(-ws.wheel_facing.y, ws.wheel_facing.x);
        auto divisor = pos.x * direction.y - pos.y * direction.x;
        
        auto lateral_force = dot_product(net_force, direction) + car.applied_torque() / divisor;
        auto longitudinal_force = (std::abs(ws.acceleration_force) + std::abs(ws.braking_force)) * pedal_adjustment;
        auto rem = std::max(ws.traction_limit * ws.traction_limit - longitudinal_force * longitudinal_force, 0.0);
        auto max_resist = std::max(std::sqrt(rem) - ws.antislide_force, 0.0);
        max_resist = clamp(ws.traction_limit * ws.cornering * (1.0 - ws.slide_ratio) - ws.antislide_force, 0.0, max_resist);
        lateral_force = clamp(lateral_force, -max_resist, max_resist);

        car.apply_force(lateral_force * direction, ws.pos);
        net_force += lateral_force * direction;
      }

      {
        handling_state.load_transfer = -net_force.y * handling.load_transfer;
      }
      return handling_state;
    }
  }
}