/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "car_hash.hpp"

#include "utility/string_utilities.hpp"

namespace ts
{
  namespace resources
  {
    struct CarDescription
    {
      std::string name;
      CarHash hash;
    };

    // Unique car identifier.
    struct CarDescriptionRef
    {
      CarDescriptionRef(boost::string_ref name_, CarHash hash_)
        : name(name_),
          hash(hash_)
      {
      }

      CarDescriptionRef(const CarDescription& car_desc)
        : name(car_desc.name), 
          hash(car_desc.hash)
      {
      }

      explicit operator CarDescription() const
      {
        CarDescription result;
        result.name.assign(name.begin(), name.end());
        result.hash = hash;
        return result;
      }

      boost::string_ref name;
      CarHash hash;
    };

    inline bool operator==(const CarDescriptionRef& a, const CarDescriptionRef& b)
    {
      return a.name == b.name && a.hash == b.hash;
    }

    inline bool operator!=(const CarDescriptionRef& a, const CarDescriptionRef& b)
    {
      return !(a == b);
    }
  }
}
