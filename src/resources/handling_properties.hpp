/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace resources
  {
    struct HandlingProperties
    {
      double engine_force = 0.0;
      double braking_force = 0.0;

      double active_cornering = 1.0;
      double reactive_cornering = 1.0;

      double drag_coefficient = 1.0;
      double downforce_coefficient = 0.0;
      double downforce_balance = 0.5;
      double rolling_coefficient = 0.0;

      // Traction limit
      double traction_limit_exponent = 1.0;
      double traction_limit_factor = 0.0;

      double weight_distribution = 0.5;

      double max_engine_revs = 250.0;
      double reverse_gear_ratio = 3.0;      
    };
  }
}