/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <random>
#include <limits>

namespace ts
{
  namespace utility
  {
    namespace detail
    {
      std::mt19937_64& random_engine();
    }

    template <typename T>
    T random_integer(T min, T max)
    {
      std::uniform_int_distribution<T> dist(min, max);
      return dist(detail::random_engine());
    }

    template <typename T>
    T random_integer()
    {
      return random_integer(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    }

    template <typename T>
    T random_real(T min, T max)
    {
      std::uniform_real_distribution<T> dist(min, max);
      return dist(detail::random_engine());
    }

    template <typename Distribution>
    auto random_number(Distribution&& dist)
    {
      return dist(detail::random_engine());
    }
  }
}
