/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/color.hpp"

#include <cstdint>
#include <array>

namespace ts
{
  namespace resources
  {
    struct ColorScheme
    {
      std::uint32_t scheme_id = 0;
      std::array<Colorb, 3> colors;
    };
  }
}
