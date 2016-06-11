/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef VECTOR3_HPP_8192384128934
#define VECTOR3_HPP_8192384128934

#include <cstdint>

namespace ts
{
  template <typename T>
  struct Vector3
  {
    T x, y, z;

    Vector3& operator+=(const Vector3& other)
    {
      x += other.x;
      y += other.y; 
      z += other.z;
      return *this;
    }
  };


  using Vector3f = Vector3<float>;
  using Vector3u = Vector3<std::uint32_t>;
}

#endif