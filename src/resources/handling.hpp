/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <boost/container/small_vector.hpp>

#include <cstdint>
#include <array>

namespace ts
{
  namespace resources
  {
    struct Handling
    {
      // Units
      // u = Unit of distance, equal to 1 track pixel.
      // m = Unit of weight
      // s = 1 second.
      // F = 1 u/s²·m, kF = 1000 F -- force
      // V = 1 u/s -- speed
      // A = 1 u/s² -- acceleration
      // rad = radians

      // Maximum acceleration force, in F
      double max_acceleration_force = 0.0;

      // Maximum braking force, in F. Note that this is per wheel.
      double max_braking_force = 0.0;

      // Speed that corresponds to max revolutions at gear ratio 1.0
      double max_engine_revs = 300.0;

      // Gear ratios
      boost::container::small_vector<double, 8> gear_ratios = {};
      double reverse_gear_ratio = 2.3;

      // Number of frames that it takes to shift gears
      int gear_shift_duration = 3;

      // drag (F) = drag_coefficient * speed² (u/s)
      double drag_coefficient = 0.13;

      // downforce (F) = downforce_coefficient * speed² (u/s)
      double downforce_coefficient = 0.21;

      // rolling_drag = vertical_load * rolling_drag_coefficient
      double rolling_drag_coefficient = 0.03;

      // Traction limit (F) defines the maximum force that all wheels put together can handle.
      // This is divided between all wheels.
      double traction_limit = 53000.0;

      // Load transfer defines how much of the acceleration/retardation force is being transferred to the 
      // rear/front of the car.
      double load_transfer = 0.14;

      // Balance variables: balance of X means X at the front, and (1.0 - X) at the rear.
      double brake_balance = 0.47;
      double downforce_balance = 0.59;
      double steering_balance = 0.0;

      // Cornering: how much of any wheel's traction limit is available for cornering.
      double cornering = 1.0;

      // Maximum steering angle in degrees. Increase this to increase the minimum turning radius,
      // and also to make it less likely the car spins out of control.
      double max_steering_angle = 30.0;

      // Slip angles in degrees at which the car is not/fully sliding.
      double non_slide_angle = 4.5;
      double full_slide_angle = 8.5;

      // For a fully sliding wheel, traction limit is multiplied by sliding_grip.
      double sliding_grip = 0.87;

      // The amount the angular velocity is decreased by every second.
      // 1.0 means the angular velocity is reduced by 2% every 20ms.
      double angular_damping = 1.3;

      // Wheel positions have an effect on the car's willingness to rotate.
      // Longer wheelbase means more torque is applied to the car, making it spin more easily.
      double wheelbase_length = 18.0;
      double wheelbase_offset = 0.0;
      double num_front_wheels = 2;
      double num_rear_wheels = 2;
      double front_axle_width = 6.0;
      double rear_axle_width = 6.0;

      // Whether the car is driven at the front, rear or both.
      bool front_driven = false;
      bool rear_driven = true;
    };
  }
}
