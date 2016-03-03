/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "car_description.hpp"
#include "car_definition.hpp"

namespace ts
{
  namespace resources
  {
    CarDescription car_description(const CarDefinition& car_def)
    {
      CarDescription car_desc;
      car_desc.name = car_def.car_name;
      car_desc.hash = car_def.car_hash;
      return car_desc;
    }
  }
}