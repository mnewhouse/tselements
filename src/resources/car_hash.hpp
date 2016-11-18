/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <array>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    struct CarDefinition;
    using CarHash = std::array<std::uint32_t, 4>;

    CarHash calculate_car_hash(const CarDefinition& car_definition);
  }
}
