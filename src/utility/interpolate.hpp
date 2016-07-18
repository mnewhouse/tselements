/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef INTERPOLATE_HPP_5571872385
#define INTERPOLATE_HPP_5571872385

namespace ts
{
  template <typename V, typename T>
  auto interpolate_linearly(const V& a, const V& b, T t)
  {
    return a * static_cast<T>(1.0 - t) + b * t;
  }
}

#endif