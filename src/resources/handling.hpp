/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef HANDLING_HPP_41189187391
#define HANDLING_HPP_41189187391

namespace ts
{
  namespace resources
  {
    struct Handling
    {
      double torque = 0.0;
      double braking = 0.0;
      double steering = 0.0;
      double grip = 0.0;
      double turn_in = 0.0;
      double antislide = 0.0;
      double traction_limit = 0.0;
      
      double drag_coefficient = 1.0;
      double rolling_coefficient = 1.0;
      double downforce_coefficient = 0.0;

      double mass = 500.0;
      double max_engine_revs = 250.0;
      double torque_multiplier = 1.0;

      struct TractionLossBehavior
      {
        double antislide_reduction = 0.0;
        double turn_in_reduction = 0.0;
        double steering_reduction = 0.0;
        double torque_reduction = 0.0;
        double braking_reduction = 0.0;
      };

      struct StressFactor
      {
        double front = 1.0;
        double neutral = 1.0;
        double back = 1.0;
      };

      double load_balance_limit = 0.0;
      double balance_shift_factor = 0.0;

      StressFactor torque_stress;
      StressFactor braking_stress;
      StressFactor turning_stress;

      TractionLossBehavior lock_up_behavior;
      TractionLossBehavior wheel_spin_behavior;
      TractionLossBehavior slide_behavior;
    };
  }
}

#endif