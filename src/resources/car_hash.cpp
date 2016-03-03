/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "car_hash.hpp"

#include "utility/sha256.hpp"

namespace ts
{
  namespace resources
  {
    CarHash calculate_car_hash(const CarDefinition& car_definition)
    {
      hash::SHA256 hash;

      auto result = hash();
      return
      {
        result[0] ^ result[3],
        result[1] ^ result[2],
        result[4] ^ result[7],
        result[5] ^ result[6]
      };
    }
  }
}