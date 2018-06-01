/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  template <typename V, typename T>
  auto interpolate_linearly(const V& a, const V& b, T t)
  {
    return a * static_cast<T>(1.0 - t) + b * t;
  }
}
