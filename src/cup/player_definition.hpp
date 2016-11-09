/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef PLAYER_DEFINITION_HPP_2298213895
#define PLAYER_DEFINITION_HPP_2298213895

#include <string>
#include <cstdint>

#include "resources/color_scheme.hpp"

namespace ts
{
  namespace cup
  {
    struct PlayerDefinition
    {
      std::string name;
      std::uint64_t id;
      std::uint16_t control_slot;

      resources::ColorScheme color_scheme;
    };
  }
}

#endif