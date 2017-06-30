/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector3.hpp"

#include <boost/container/small_vector.hpp>
#include <boost/optional.hpp>

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

      // Base torque, in kF.
      float torque = 0.0f;

      // Extra torque at low speeds.
      float extra_torque = 0.0f;

      // Maximum speed, with gear ratio 1.0.
      float max_engine_revs = 300.0f;

      // Maximum braking force in kF.
      float braking = 0.0f;

      // Maximum steering speed, in rad/s
      float steering = 0.0f; 

      // Maximum cornering force, in kF.
      float cornering = 0.0f;

      // Maximum antislide force, in kF.
      float antislide = 0.0f;

      // Traction limit, in kF.
      float traction_limit = 0.0f;

      // Minimum turning radius, in u.
      float min_turning_radius = 0.0f;

      // Drag coefficient, in F/V².
      float drag_coefficient = 1.0f;

      // Rolling resistance, in kF.
      float rolling_resistance = 0.0f;

      // How much of the air resistance is translated into downforce.
      float downforce_coefficient = 0.0f;

      struct DownforceEffect
      {
        float traction_limit = 1.0f;
        float braking = 1.0f;
        float cornering = 1.0f;
        float antislide = 1.0f;
        float rolling_resistance = 0.0f;
      } downforce_effect;

      struct StressFactor
      {
        float torque = 1.0f;
        float braking = 1.0f;
        float cornering = 1.0f;
        float antislide = 1.0f;
      } stress_factor;

      float sliding_traction = 1.0f;

      // Limit the input controls based on the traction limit.
      float input_moderation = 1.0f;
      float steering_bias = 0.666f;

      // Mass affects how much impact the various forces have on the velocity of the car.
      float mass = 100.0f;

      // Mass distribution affects how much weight is situated at the front and rear.
      // 0.5 = evenly balanced, <0.5 = less at the front, >0.5 = more at the front.
      float mass_distribution = 0.5f;

      // How quickly load transfers from front to back.
      float load_transfer = 0.0f;

      // The maximum load tranferred to the front or back, in kF.
      float load_balance_limit = 0.0f;
      
      float load_balance_cornering_stress_effect = 0.0f;
      float load_balance_antislide_stress_effect = 0.0f;
      float load_balance_torque_stress_effect = 0.0f;
      float load_balance_braking_stress_effect = 0.0f;



      struct Wheel
      {
        enum Axle
        {
          Front, Rear
        } axle;

        Vector2f position;
      };

      boost::container::small_vector<Wheel, 4> wheel_positions;

      float reverse_gear_ratio = 2.0f;
    };

    struct RawHandlingState
    {
      std::uint32_t speed_delta;
      std::uint32_t turning_speed;

      std::uint16_t engine_rev_speed;
      std::uint8_t current_gear;
      
      // For local usage only.
      std::uint16_t front_traction;
      std::uint16_t rear_traction;
    };

    struct HandlingState
    {
      double load_balance;
      double turning_speed;

      double engine_rev_speed;
      double traction;

      std::int32_t current_gear;
    };

    struct Handling_Realistic
    {
      // Units
      // u = Unit of distance, equal to 1 track pixel.
      // m = Unit of weight
      // s = 1 second.
      // F = 1 u/s²·m, kF = 1000 F -- force
      // V = 1 u/s -- speed
      // A = 1 u/s² -- acceleration
      // rad = radians

      float torque = 0.0f; // Maximum engine torque.
      float max_engine_revs = 300.0f; // The car's speed at full revs with a gear ratio of 1.0.

      float engine_rev_up_speed = 5.0f; // Engine can rev up from zero to max this many times per second.

      float braking = 0.0f; // Maximum braking force, shared between all wheels.

      float turning_radius = 20.0f; // Minimum turning radius, in units of distance.

      float traction_coefficient = 1.0f; // Determines the traction limit based on the vertical load on a tyre.
      float cornering_force = 1.0f; // Ratio of traction limit available for cornering.
      float slide_friction = 0.6f; // When a tyre is sliding, friction coefficient is multiplied by this factor.

      float drag_coefficient = 1.0f; // Determines air resistance: F = velocity * speed * drag_coefficient
      float rolling_resistance = 0.03f; // F = FNormal * rolling_resistance
      float downforce_coefficient = 0.0f; // How much of the air resistance is translated into downforce.

      float peak_slip_angle = 10.0f; // Slip angle (degrees) with optimal cornering force.

      float mass = 500.0f; // Mass of the car. Determines the impact of all forces on the velocity of the car.
      float moment_of_inertia = 1.0f; 

      float throttle_control = 1.0f; // Limits the throttle according to available traction. 
      float braking_control = 1.0f; // Limits the force of the brakes according to available traction.
      float steering_control = 1.0f; // Adjusts the steering angle of the wheels according to available traction.
      float traction_control = 1.0f; // Prevents the wheels from slipping.

      Vector3f center_of_mass = {};

      boost::container::small_vector<float, 16> torque_curve;     

      boost::container::small_vector<float, 8> gear_ratios;
      float reverse_gear_ratio = 0.0f;

      struct Axle
      {
        bool steered = false;
        float braking = 0.5f;        
        float torque = 0.0f;
        float traction_limit = 1.0f;
        float downforce = 0.5f;

        boost::container::small_vector<Vector2f, 2> wheel_positions;
      };

      std::array<Axle, 2> axles;
    };
    
    struct HandlingState_Realistic
    {
      std::int32_t current_gear = 0;
      float engine_rev_speed = 0.0f;

      Vector2f acceleration_force = {};
      float rotational_force = 0.0f;
    };
  }
}
