/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "car.hpp"
#include "handling_v2.hpp"

#include <boost/container/small_vector.hpp>

#include "utility/transform.hpp"
/*
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
        auto transform = make_transformation(rotation);
        auto facing_vector = transform_point({ 0.0, -1.0 }, transform);

        auto inv_transform = make_transformation(-rotation);

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

        handling_state.current_gear = 1;

        auto gear = static_cast<std::uint32_t>(handling_state.current_gear);
        if (gear == 0)
        {
          torque_multiplier = 0.0f;
        }

        else if (gear <= gear_ratios.size())
        {
          torque_multiplier = gear_ratios[gear - 1];
        }

        else
        {
          torque_multiplier = -handling.reverse_gear_ratio;
        }

        auto torque_curve = handling.torque_curve;

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
        axles.resize(2);
        
        {
          auto& front = axles[0];
          auto& rear = axles[1];

          front.steering = 1.0;
          front.driven = true;
          front.braking = 0.5f;

          rear.steering = 0.0;
          rear.driven = false;
          rear.braking = 0.5;

          resources::Handling_v2::Wheel wheel;
          wheel.position = { -3.0f, -5.0f };
          front.wheels.push_back(wheel);

          wheel.position.x = -wheel.position.x;
          front.wheels.push_back(wheel);

          wheel.position.x = -wheel.position.x;
          wheel.position.y = -wheel.position.y;
          rear.wheels.push_back(wheel);

          wheel.position.x = -wheel.position.x;
          rear.wheels.push_back(wheel);
        }

        struct WheelState
        {
          double base_load;
          double load;

          Vector2d position;

          Vector2d net_force;          
          Vector2d rotational_impulse;
          double rotational_acceleration;
        };

        boost::container::small_vector<WheelState, 4> wheel_states;

        double total_load = 0.0;
        for (auto& axle : axles)
        {
          for (auto& wheel : axle.wheels)
          {
            auto load_value = 1.0 / std::max(magnitude(wheel.position - handling.center_of_mass), 1.0);            

            WheelState ws{};
            ws.load = load_value;
            wheel_states.push_back(ws);

            total_load += load_value;
          }
        }

        if (total_load >= 0.001)
        {
          auto load_multiplier = handling.mass / total_load;
          for (auto& wheel : wheel_states)
          {
            wheel.load *= load_multiplier;
            wheel.base_load = wheel.load;
          }
        }

        auto inv_force_multiplier = handling.mass / frame_duration;
        auto force_multiplier = frame_duration / handling.mass;

        auto drag_resistance = speed * -update_state.velocity * handling.drag_coefficient;
        auto downforce = dot_product(facing_vector, drag_resistance);
        if (downforce >= 0.0) downforce = 0.0;
        else downforce *= downforce * handling.downforce_coefficient;

        auto inv_num_wheels = !wheel_states.empty() ? 1.0 / wheel_states.size() : 0.0;

        DoubleRect wheel_base{};
        auto wheel_it = wheel_states.begin();
        for (const auto& axle : axles)
        {
          for (const auto& wheel : axle.wheels)
          {
            auto wheel_offset = wheel.position - handling.center_of_mass;
            auto acceleration_force = make_vector2(wheel_offset.y, -wheel_offset.x) * handling_state.rotational_acceleration_force;
            acceleration_force += wheel_offset * transform_point(handling_state.acceleration_force, inv_transform);
            acceleration_force *= inv_force_multiplier;           

            auto wheel_load = handling.mass * acceleration_force * handling.load_coefficient;
            wheel_load += downforce * inv_num_wheels * wheel.downforce;

            wheel_it->position = wheel.position;
            wheel_it->load += magnitude(wheel_load);

            if (wheel_it == wheel_states.begin()) wheel_base = { wheel.position.x, wheel.position.y, 0.0, 0.0 };
            else wheel_base = combine(wheel_base, wheel.position);

            ++wheel_it;            
          }
        }

        auto driven_axles = std::count_if(axles.begin(), axles.end(),
                                          [](const auto& axle)
        {
          return axle.driven;
        });

        auto wheel_base_center = make_vector2(wheel_base.left + wheel_base.width * 0.5,
                                              wheel_base.top + wheel_base.height * 0.5);

        wheel_it = wheel_states.begin(); 
        for (const auto& axle : axles)
        {
          auto acceleration_force_1d = 0.0;          

          if (axle.driven)
          {
            // Apply torque to the axle's wheels.
            acceleration_force_1d += torque_factor / (driven_axles * axle.wheels.size());            
          }

          auto braking_force_1d = axle.braking * braking_factor;
          auto turning_rate = steering_state * axle.steering;

          auto steering_angle = degrees(handling.max_steering_angle * turning_rate);
          auto wheel_direction = transform_point(make_vector2(0.0, -1.0), steering_angle);

          for (const auto& wheel : axle.wheels)
          {
            auto wheel_velocity = update_state.rotating_speed * make_vector2(-wheel.position.y, wheel.position.x);
            wheel_velocity += transform_point(update_state.velocity, inv_transform);
            auto wheel_heading = normalize(wheel_velocity);
 
            auto wheel_angle = radians(std::atan2(wheel_direction.x, -wheel_direction.y));
            auto wheel_heading_angle = radians(std::atan2(wheel_heading.x, -wheel_heading.y));

            auto slip_angle = wheel_heading_angle - wheel_angle;
            auto reverse_slip_angle = normalize(degrees(180.0) + slip_angle));

            if (std::abs(reverse_slip_angle.radians()) < std::abs(slip_angle.radians()))
            {
              std::swap(slip_angle, reverse_slip_angle);
            }

            // Calculate the cornering force.
            auto slip_degrees = slip_angle.degrees();

            // Passive cornering increases with slip angle, up to a certain point.

            wheel_it->net_force += (wheel_direction * acceleration_force_1d);
            wheel_it->net_force -= wheel_heading * braking_force_1d;


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

            wheel.rotational_impulse = wheel.net_force * impulse;
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
            auto apply = std::bind(apply_rotational_acceleration , std::placeholders::_1, net_rotational_acceleration / total_neg);
            std::transform(wheel_states.begin(), partition_it, wheel_states.begin(), apply);
          }

          else if (net_rotational_acceleration > 0.0)
          {
            auto apply = std::bind(apply_rotational_acceleration, std::placeholders::_1, net_rotational_acceleration / total_pos);
            std::transform(partition_it, wheel_states.end(), partition_it, apply);
          }          
        }

        Vector2d net_force = {};
        for (const WheelState& wheel : wheel_states)
        {
          net_force += wheel.net_force;
        }        

        auto revs = handling_state.engine_rev_speed;
        auto max_new_revs = std::min(revs + handling.engine_rev_up_speed * frame_duration, 1.0);

        
        auto new_velocity = update_state.velocity + transform_point(net_force, transform) * force_multiplier;
        update_state.rotating_speed += net_rotational_acceleration * frame_duration;

        auto new_speed = magnitude(new_velocity);
        auto new_drag_resistance = new_speed * -new_velocity * handling.drag_coefficient;

        new_velocity += new_drag_resistance * force_multiplier;        

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


/*
auto is_turning = abs_turning_rate >= 0.00001;
if (is_turning)
{
turning_radius = handling.min_turning_radius / abs_turning_rate;
}

auto turning_circle_center = make_vector2(turning_radius, 0.0);
if (turning_rate < 0.0) turning_circle_center.x = -turning_circle_center.x;

turning_circle_center += center;

for (const auto& wheel : axle.wheels)
{
auto wheel_direction = make_vector2(0.0, -1.0);
if (is_turning)
{
wheel_direction = normalize(flip_orientation(turning_circle_center - wheel.position));
}

auto wheel_angle = radians(std::atan2(wheel_direction.x, -wheel_direction.y));

auto traction_limit = wheel_it->load * wheel.traction;

axle_state.wheel_data.emplace_back();
WheelData& wheel_data = axle_state.wheel_data.back();

auto rotating_speed = make_vector2(-wheel_data.position.y,
wheel_data.position.x) * update_state.rotating_speed;

auto wheel_velocity = transform_point(update_state.velocity, inv_transform) + rotating_speed;
auto wheel_heading = normalize(wheel_velocity);
auto wheel_heading_angle = radians(std::atan2(wheel_velocity.x, -wheel_velocity.y));

auto slip_angle = normalize(wheel_angle - wheel_heading_angle);
auto reverse_slip_angle = normalize(wheel_angle - (wheel_heading_angle - degrees(180.0)));

if (std::abs(reverse_slip_angle.radians()) < std::abs(slip_angle.radians()))
{
std::swap(reverse_slip_angle, slip_angle);
}

auto slip_degrees = slip_angle.degrees();
auto acceleration_force = wheel_direction * acceleration_force_1d;

// Limit steering/acceleration/braking - TODO for later
// First step: apply acceleration force


wheel_data.net_force += acceleration_force;

++wheel_it;
}
*/


/*
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
*/