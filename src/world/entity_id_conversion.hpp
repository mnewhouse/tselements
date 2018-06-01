/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "entity.hpp"

#include <cstdint>

namespace ts
{
  namespace world
  {
    inline EntityId car_id_to_entity_id(std::uint8_t car_id)
    {
      return car_id;
    }

    inline std::uint8_t entity_id_to_car_id(EntityId entity_id)
    {
      return static_cast<std::uint8_t>(entity_id & 0xFF);
    }
  }
}
