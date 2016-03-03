/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef ENTITY_ID_CONVERSION_HPP_13458198512
#define ENTITY_ID_CONVERSION_HPP_13458198512

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

#endif