/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector3.hpp"

#include "resources/terrain_definition.hpp"

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
      double torque = 24000.0;

      // Maximum speed, with gear ratio 1.0.
      double max_engine_revs = 300.0f;

      // Maximum braking force in kF.
      double braking = 12000.0f;

      double peak_slip_angle = 10.0;
      double max_steering_angle = 10.0;      
      
      // Traction limit = (tyre_load ^ traction_limit_exponent) * traction_limit_factor.
      double traction_limit_factor = 2.0f;     
      double traction_limit_exponent = 0.9f;

      // Mass affects how much impact the various forces have on the velocity of the car.
      double mass = 500.0f;

      // Center of mass. Impacts the base load on all tyres, and affects how easily various parts of the car
      // will rotate.      
      Vector2d center_of_mass = {};

      // Drag coefficient, in F/V².
      double drag_coefficient = 0.05f;

      // How much of the air resistance is translated into downforce.
      double downforce_coefficient = 0.3f;

      // How much of the vertical tyre load is turned into rolling resistance
      double rolling_resistance_factor = 0.0f;

      // How much the accelerative forces affect the car's load on the tyres.
      Vector2d load_transfer_coefficient = {};

      // Limit the input controls based on the traction limit.
      double input_moderation = 1.0f;
      double steering_bias = 0.666f;

      double slide_friction = 0.7;

      struct Axle
      {
        bool driven = false; // Whether this axle is driven by the engine
        double braking = 1.0f; // Braking force factor * base braking is multiplied by this number
       
        double cornering = 0.0f; // Cornering, as a fraction of the traction limit for each tyre. 
                                 // If zero, no active cornering on this axle.
        double antislide = 0.0f; // Antislide, or passive cornering. As a fraction of the traction limit.
        double downforce = 1.0f;

        boost::container::small_vector<Vector2d, 2> wheels;
      };

      std::array<Axle, 2> axles;
      boost::container::small_vector<double, 8> gear_ratios;

      double reverse_gear_ratio = 3.0f;
    };

    struct RawHandlingState
    {     
      std::uint16_t engine_rev_speed;
      std::uint16_t lateral_acceleration;
      std::uint16_t longitudinal_acceleration;
      std::uint16_t rotational_acceleration;
      
      std::uint8_t current_gear;
    };

    struct HandlingState
    { 
      double engine_rev_speed;
      Vector2d acceleration;
      double rotational_acceleration;
      std::int32_t current_gear;

      // For local usage only.
      struct WheelState
      {
        resources::TerrainDefinition current_terrain;
        double traction;
      };

      boost::container::small_vector<WheelState, 4> current_terrains;
    };
  }
}
