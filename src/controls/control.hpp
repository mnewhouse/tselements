/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CONTROL_HPP_13890128
#define CONTROL_HPP_13890128

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
      Accelerate = 1,
      Brake = 2,
      Left = 4,
      Right = 8,
      Fire = 16,
      AltFire = 32
    };

    // A few utility functions to facilitate working with control bitmasks.
    inline std::uint16_t& operator|=(std::uint16_t& mask, Control control)
    {
      return mask |= static_cast<std::uint16_t>(control);
    }

    inline std::uint16_t operator|(std::uint16_t mask, Control control)
    {
      return mask |= control;
    }

    inline std::uint16_t& operator^=(std::uint16_t& mask, Control control)
    {
      return mask ^= static_cast<std::uint16_t>(control);
    }

    inline std::uint16_t operator^(std::uint16_t mask, Control control)
    {
      return mask |= control;
    }

    inline std::uint16_t& operator&=(std::uint16_t& mask, Control control)
    {
      return mask &= static_cast<std::uint16_t>(control);      
    }

    inline std::uint16_t operator&(std::uint16_t mask, Control control)
    {
      return mask &= control;
    }
  }
}

#endif