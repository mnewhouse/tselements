/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <functional>

namespace ts
{
  template <typename T, typename Comp>
  auto clamp(const T& a, decltype(a) min, decltype(a) max, Comp&& comp)
  {
    if (comp(a, min)) return min;
    if (comp(max, a)) return max;
      
    return a;
  }


  template <typename T>
  auto clamp(const T& a, decltype(a) min, decltype(a) max)
  {
    return clamp(a, min, max, std::less<>{});
  }

  template <typename T>
  auto next_power_of_two(T n)
  {
    T result = 1;
    while (result < n) result <<= 1;

    return result;
  }
}