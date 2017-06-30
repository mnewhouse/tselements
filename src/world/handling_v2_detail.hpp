/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "handling_v2.hpp"

#include <boost/container/small_vector.hpp>

#include "utility/debug_log.hpp"

namespace ts
{
  namespace world
  {
    namespace handling_v2
    {
      template <typename TerrainLookupFunc>
      UpdateState update_car_state(const Car& car, TerrainLookupFunc&& terrain_lookup,
                                   double frame_duration)
      {
        UpdateState update_state;
        update_state.handling_state = car.handling_state();
        update_state.velocity = car.velocity();
        update_state.rotating_speed = car.rotating_speed();

        auto heading = normalize(update_state.velocity);       

        auto rotation = car.rotation();
        auto speed = magnitude(update_state.velocity);

        auto& handling_state = update_state.handling_state;
        const auto& handling = car.handling();

        using controls::Control;
        auto throttle_state = car.control_state(Control::Throttle) / 255.0;
        auto braking_state = car.control_state(Control::Brake) / 255.0;
        auto steering_state = (car.control_state(Control::Right) - car.control_state(Control::Left)) / 255.0;

        auto torque_multiplier = 1.0;

        auto gear_ratios = handling.gear_ratios;
        gear_ratios = { 3.0f, 2.2f, 1.7f, 1.4f, 1.2f, 1.0f };

        auto gear = static_cast<std::uint32_t>(handling_state.current_gear) + 1;
        if (gear == 0)
        {
          torque_multiplier = 0.0f;
        }

        else if (gear <= gear_ratios.size())
        {
          torque_multiplier = gear_ratios[gear - 1];
        }

        auto torque_curve = handling.torque_curve;
        torque_curve = { 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.88f, 0.95f, 1.0f, 0.95f, 0.88f, 0.8f, 0.7f };

        if (!torque_curve.empty())
        {
          auto curve_position = handling_state.engine_rev_speed * (torque_curve.size() - 1);
          auto index = static_cast<std::size_t>(curve_position);

          if (index >= torque_curve.size() - 1)
          {
            // Use the last entry in the torque curve.
            torque_multiplier *= torque_curve.back();
          }

          else
          {
            // Otherwise, interpolate between two adjacent curve entries, based on the fractional part of the curve position.
            auto p = curve_position - std::floor(curve_position);
            auto a = torque_curve[index], b = torque_curve[index + 1];

            torque_multiplier *= a * (1 - p) + b * p;
          }
        }

        auto torque_factor = throttle_state * handling.torque * torque_multiplier;
        auto braking_factor = braking_state * handling.braking;

        auto axles = handling.axles;
        
        {
          auto& front = axles[0];
          auto& rear = axles[1];

          front.steering = 1.0;
          front.driven = false;
          front.braking = 0.4f;

          rear.steering = 0.0;
          rear.driven = true;
          rear.braking = 0.6;

          resources::Handling_v2::Wheel wheel;
          wheel.position = { -3.0f, -3.0f };
          front.wheels.push_back(wheel);

          wheel.position.x = -wheel.position.x;
          front.wheels.push_back(wheel);

          wheel.position.x = -wheel.position.x;
          wheel.position.y = -wheel.position.y;
          rear.wheels.push_back(wheel);

          wheel.position.x = -wheel.position.x;
          rear.wheels.push_back(wheel);
        }

        auto driven_axles = std::count_if(axles.begin(), axles.end(),
                                          [](const auto& axle)
        {
          return axle.driven;
        });        

        struct AxleState
        {
          struct Wheel
          {
            Vector2d net_force;
            Vector2d position;
          };

          boost::container::small_vector<Wheel, 2> wheel_data;
        };

        auto inv_force_multiplier = handling.mass / frame_duration;
        auto force_multiplier = frame_duration / handling.mass;

        auto axle_braking = braking_factor / axles.size();

        boost::container::small_vector<AxleState, 2> axle_states;
        for (const auto& axle : axles)
        {
          auto steering_angle = degrees(steering_state * axle.steering * handling.steering_lock);
          auto wheel_angle = rotation + steering_angle;
          auto wheel_direction = transform_point({ 0.0f, -1.0f }, wheel_angle);          

          auto acceleration_force_1d = 0.0;          

          if (axle.driven)
          {
            // Apply torque to the axle's wheels.
            acceleration_force_1d += torque_factor / (driven_axles * axle.wheels.size());            
          }

          axle_states.emplace_back();
          auto& axle_state = axle_states.back();

          auto acceleration_force = wheel_direction * acceleration_force_1d;
          for (const auto& wheel : axle.wheels)
          {
            auto traction_limit = handling.traction_limit * wheel.traction; // TODO: Load on wheel            

            axle_state.wheel_data.emplace_back();
            auto& wheel_data = axle_state.wheel_data.back();
            wheel_data.position = transform_point(vector2_cast<double>(wheel.position), rotation);            

            auto rotating_speed = make_vector2(-wheel_data.position.y, 
                                               wheel_data.position.x) * update_state.rotating_speed;

            auto wheel_velocity = update_state.velocity + rotating_speed;
            auto wheel_heading = normalize(wheel_velocity);

            auto wheel_heading_angle = radians(std::atan2(wheel_velocity.x, -wheel_velocity.y));

            auto slip_angle = normalize(wheel_angle - wheel_heading_angle);
            auto reverse_slip_angle = normalize(wheel_angle - (wheel_heading_angle - degrees(180.0)));

            if (std::abs(reverse_slip_angle.radians()) < std::abs(slip_angle.radians())) 
            {
              slip_angle = reverse_slip_angle;
            }

            // Calculate the cornering force we would get with full grip.
            auto slip_degrees = slip_angle.degrees();
            auto slip_angle_ratio = std::abs(slip_degrees / handling.peak_slip_angle);

            auto max_retardation_force = magnitude(wheel_velocity * inv_force_multiplier);            

            auto braking_force = axle_braking * axle.braking;

            auto slip_resistance_1d = std::abs(slip_degrees / 90.0) * traction_limit * 0.2;
            auto slip_resistance = -wheel_heading * std::min(max_retardation_force, slip_resistance_1d + braking_force);

            auto max_cornering_force = traction_limit;
            auto cornering_force_factor = std::max(-slip_angle_ratio * slip_angle_ratio + 2.0 * slip_angle_ratio, 0.0);

            auto cornering_force_direction = make_vector2(-wheel_direction.y, wheel_direction.x);
            if (dot_product(cornering_force_direction, wheel_velocity) >= 0.0)
            {
              cornering_force_direction = -cornering_force_direction;
            }

            auto cornering_force = cornering_force_direction * cornering_force_factor * max_cornering_force;
            wheel_data.net_force = acceleration_force + cornering_force + slip_resistance;
          }
        }

        auto revs = handling_state.engine_rev_speed;
        auto max_new_revs = std::min(revs + handling.engine_rev_up_speed * frame_duration, 1.0);

        Vector2d net_force = {};
        if (!axle_states.empty())
        {
          Vector2d pivot_point = {};

          // Now, take all the wheels' forces and apply them to the velocity of the car.
          // Forces also affect the rotational speed. 

          struct WheelForce
          {
            double rotation; // The rotational speed that we can maximally get from a wheel
            Vector2<double> used_force; // The force that is used for that rotational speed.
          };

          boost::container::small_vector<WheelForce, 8> wheel_forces;
          double net_rotation = 0.0;

          for (const auto& axle : axle_states)
          {
            for (const auto& wheel : axle.wheel_data)
            {
              auto offset = wheel.position - pivot_point;              

              auto spin_direction = normalize(make_vector2(-offset.y, offset.x));
              auto force_direction = normalize(wheel.net_force);

              auto spin_factor = dot_product(spin_direction, force_direction);
              spin_factor *= std::abs(spin_factor);

              WheelForce wheel_force;
              wheel_force.used_force = wheel.net_force * std::abs(spin_factor);
              wheel_force.rotation = magnitude(wheel.net_force) * spin_factor / (0.3 * magnitude(offset) * 6.28318530718);
              wheel_forces.push_back(wheel_force);

              net_force += wheel.net_force;
              net_rotation += wheel_force.rotation;
            }
          }
          
          auto partition_end = std::partition(wheel_forces.begin(), wheel_forces.end(),
                                              [=](const WheelForce& wf)
          {
            return wf.rotation >= 0.0;
          });

          auto applied = boost::make_iterator_range(wheel_forces.begin(), partition_end);
          auto cancelled = boost::make_iterator_range(partition_end, wheel_forces.end());
          
          if (net_rotation < 0.0)
          {
            using std::swap;
            swap(cancelled, applied);            
          }

          // Find the wheels whose rotational impacts were cancelled out.
          auto cancelled_rotation = std::accumulate(cancelled.begin(), cancelled.end(), 0.0,
                                                    [](auto total, const auto& wf)
          {
            return total + wf.rotation;
          });

          if (std::abs(net_rotation) > 0.001)
          {            
            auto applied_ratio = std::abs(net_rotation / (net_rotation - cancelled_rotation));

            for (const auto& wheel_force : applied)
            {
              net_force -= wheel_force.used_force * applied_ratio;
            }
          }

          update_state.rotating_speed += net_rotation * force_multiplier;
        }

        
        auto new_velocity = update_state.velocity + net_force * force_multiplier;

        auto new_speed = magnitude(new_velocity);
        auto drag_resistance = new_speed * -new_velocity * handling.drag_coefficient;

        new_velocity += drag_resistance * force_multiplier;        

        if (dot_product(new_velocity, update_state.velocity) < 0.0)
        {
          //new_velocity = { 0.0, 0.0 };
        }

        update_state.velocity = new_velocity;

        return update_state;
      }
    }
  }
}