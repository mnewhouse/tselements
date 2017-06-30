/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>

namespace ts
{
  namespace controls
  {
    // Simple enum class that contains a list of control identifiers.
    // These must be usable in a bitmask, so the values are all powers of two.
    enum class Control : std::uint16_t
    {
      None = 0,
      Throttle = 1,
      Brake = 2,
      Left = 4,
      Right = 8,
      Fire = 16,
      AltFire = 32
    };

    enum class FreeControl : std::uint16_t
    {
      None = 0,
      Throttle = 1,
      Brake = 2,
      Left = 4,
      Right = 8
    };

    struct ControlsMask
    {
      std::uint8_t left = 0;
      std::uint8_t right = 0;
      std::uint8_t throttle = 0;
      std::uint8_t brake = 0;
      std::uint16_t other = 0;
    };
  }
}
