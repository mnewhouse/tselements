/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "vector2.hpp"

#include <iosfwd>
#include <cstdint>
#include <type_traits>

namespace ts
{
  template <typename T>
  struct Vector3
  {
    Vector3() = default;

    explicit Vector3(T v)
      : x(v), y(v), z(v)
    {      
    }

    Vector3(T x_, T y_, T z_)
      : x(x_), y(y_), z(z_)
    {}

    Vector3& operator+=(const Vector3& other)
    {
      x += other.x;
      y += other.y; 
      z += other.z;
      return *this;
    }

    Vector3& operator-=(const Vector3& other)
    {
      x -= other.x;
      y -= other.y;
      z -= other.z;
      return *this;
    }

    Vector3& operator+=(const T& a)
    {
      return *this += Vector3(a);
    }

    Vector3& operator*=(const Vector3& other)
    {
      x *= other.x;
      y *= other.y;
      z *= other.z;
      return *this;
    }

    Vector3& operator*=(const T& m)
    {
      return *this *= Vector3(m);
    }

    Vector3& operator/=(const Vector3& q)
    {
      x /= q.x;
      y /= q.y;
      z /= q.z;
      return *this;
    }

    Vector3& operator/=(const T& q)
    {
      return *this /= Vector3(q);
    }

    T x = {};
    T y = {};
    T z = {};
  };

  template <typename T, typename U>
  auto operator+(Vector3<T> lhs, const U& rhs) -> std::remove_reference_t<decltype(lhs += rhs)>
  {
    return lhs += rhs;
  }

  template <typename T, typename U>
  auto operator-(Vector3<T> lhs, const U& rhs) -> std::remove_reference_t<decltype(lhs -= rhs)>
  {
    return lhs -= rhs;
  }

  template <typename T, typename U>
  auto operator*(Vector3<T> lhs, const U& rhs) -> std::remove_reference_t<decltype(lhs *= rhs)>
  {
    return lhs *= rhs;
  }

  template <typename T, typename U>
  auto operator/(Vector3<T> lhs, const U& rhs) -> std::remove_reference_t<decltype(lhs /= rhs)>
  {
    return lhs /= rhs;
  }

  template <typename T>
  Vector3<T> operator-(const Vector3<T>& vec)
  {
    return{ -vec.x, -vec.y, -vec.z };
  }

  template <typename T>
  auto magnitude(const Vector3<T>& v)
  {
    using std::sqrt;
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  }

  template <typename T>
  auto distance(const Vector3<T>& a, const Vector3<T>& b)
  {
    return magnitude(b - a);
  }

  template <typename T>
  auto normalize(const Vector3<T>& vec)
  {
    auto mag = magnitude(vec);
    if (mag == 0) return vec;

    return vec / mag;
  }

  template <typename T>
  auto dot_product(const Vector3<T>& a, const Vector3<T>& b)
  {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }

  template <typename T>
  auto cross_product(const Vector3<T>& a, const Vector3<T>& b)
  {
    return Vector3<T>(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y);
  }

  template <typename T>
  auto make_vector3(T x, T y, T z)
  {
    return Vector3<T>(x, y, z);
  }

  template <typename T>
  auto make_2d(Vector3<T> vec)
  {
    return Vector2<T>(vec.x, vec.y);
  }

  template <typename T>
  auto make_3d(Vector3<T> vec)
  {
    return vec;
  }

  template <typename To, typename From>
  auto vector3_cast(const Vector3<From>& v)
  {
    return make_vector3(static_cast<To>(v.x), static_cast<To>(v.y), static_cast<To>(v.z));
  }

  template <typename T>
  bool operator==(const Vector3<T>& a, const Vector3<T>& b)
  {
    return a.x == b.x && a.y == b.y && a.z == b.z;
  }

  template <typename T>
  bool operator!=(const Vector3<T>& a, const Vector3<T>& b)
  {
    return !(a == b);
  }

  using Vector3f = Vector3<float>;
  using Vector3u = Vector3<std::uint32_t>;
  using Vector3i = Vector3<std::int32_t>;

  template <typename T>
  std::ostream& operator<<(std::ostream& stream, const Vector3<T>& v)
  {
    return stream << v.x << ", " << v.y << ", " << v.z;
  }
}

