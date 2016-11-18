/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "vector3.hpp"

#include <cstdint>
#include <cmath>
#include <iosfwd>

namespace ts
{
  template <typename T>
  struct Vector2
  {
  public:
    Vector2()
      : x(), y()
    {}

    Vector2(T x, T y)
      : x(x), y(y)
    {}

    template <typename U>
    explicit Vector2(Vector2<U> other)
      : x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
    {}

    Vector2<T>& operator+=(Vector2<T> other)
    {
      x += other.x;
      y += other.y;
      return *this;
    }

    Vector2<T>& operator+=(T num)
    {
      x += num;
      y += num;
      return *this;
    }

    Vector2<T>& operator-=(Vector2<T> other)
    {
      x -= other.x;
      y -= other.y;
      return *this;
    }

    Vector2<T>& operator-=(T num)
    {
      x -= num;
      y -= num;
      return *this;
    }

    Vector2<T>& operator*=(Vector2<T> other)
    {
      x *= other.x;
      y *= other.y;
      return *this;
    }

    Vector2<T>& operator*=(T num)
    {
      x *= num;
      y *= num;
      return *this;
    }

    Vector2<T>& operator/=(Vector2<T> other)
    {
      x /= other.x;
      y /= other.y;
      return *this;
    }


    Vector2<T>& operator/=(T num)
    {
      x /= num;
      y /= num;
      return *this;
    }

    T x;
    T y;
  };

  template <typename T, typename U>
  auto operator+(Vector2<T> a, Vector2<U> b)
  {
    return make_vector2(a.x + b.x, a.y + b.y);
  }

  template <typename T, typename U>
  auto operator-(Vector2<T> a, Vector2<U> b)
  {
    return make_vector2(a.x - b.x, a.y - b.y);
  }

  template <typename T, typename U>
  auto operator*(Vector2<T> a, Vector2<U> b)
  {
    return make_vector2(a.x * b.x, a.y * b.y);
  }

  template <typename T, typename U>
  auto operator/(Vector2<T> a, Vector2<U> b)
  {
    return make_vector2(a.x / b.x, a.y / b.y);
  }

  template <typename T, typename U>
  auto operator+(Vector2<T> vec, U num)
  {
    return make_vector2(vec.x + num, vec.y + num);
  }

  template <typename T, typename U>
  auto operator+(T num, Vector2<U> vec)
  {
    return make_vector2(num + vec.x, num + vec.y);
  }

  template <typename T, typename U>
  auto operator-(Vector2<T> vec, U num)
  {
    return make_vector2(vec.x - num, vec.y - num);
  }

  template <typename T, typename U>
  auto operator-(T num, Vector2<U> vec)
  {
    return make_vector2(num - vec.x, num - vec.y);
  }

  template <typename T, typename U>
  auto operator*(Vector2<T> vec, U num)
  {
    return make_vector2(vec.x * num, vec.y * num);
  }

  template <typename T, typename U>
  auto operator*(T num, Vector2<U> vec)
  {
    return make_vector2(num * vec.x, num * vec.y);
  }

  template <typename T, typename U>
  auto operator/(Vector2<T> vec, U num)
  {
    return make_vector2(vec.x / num, vec.y / num);
  }

  template <typename T, typename U>
  auto operator/(T num, Vector2<U> vec)
  {
    return make_vector2(num / vec.x, num / vec.y);
  }

  template <typename T>
  auto operator-(Vector2<T> vec)
  {
    return make_vector2(-vec.x, -vec.y);
  }

  template <typename T>
  bool operator==(const Vector2<T>& a, const Vector2<T>& b)
  {
    return a.x == b.x && a.y == b.y;
  }

  template <typename T>
  bool operator!=(const Vector2<T>& a, const Vector2<T>& b)
  {
    return !(a == b);
  }

  template <typename T>
  T magnitude(const Vector2<T>& vec)
  {
    using std::abs;
    using std::hypot;

    if (vec.x == 0.0) return abs(vec.y);
    if (vec.y == 0.0) return abs(vec.x);

    return std::hypot(vec.x, vec.y);
  }

  template <typename T, typename U>
  auto dot_product(const Vector2<T>& a, const Vector2<U>& b)
  {
    return a.x * b.x + a.y * b.y;
  }

  template <typename T>
  auto normalize(Vector2<T> vec)
  {
    auto mag = magnitude(vec);
    if (mag == 0) return vec;

    return vec /= mag;
  }

  template <typename T>
  auto distance(const Vector2<T>& a, const Vector2<T>& b)
  {
    using std::hypot;
    return hypot(a.x - b.x, a.y - b.y);
  }

  template <typename T>
  auto flip_orientation(Vector2<T> vec)
  {
    using std::swap;
    swap(vec.x, vec.y);
    return vec;
  }

  template <typename To, typename From>
  auto vector2_cast(Vector2<From> vec)
  {
    return make_vector2(static_cast<To>(vec.x), static_cast<To>(vec.y));
  }
  
  template <typename To, typename From>
  auto vector2_round(const Vector2<From>& vec)
  {
    using std::round;
    return make_vector2(static_cast<To>(round(vec.x)), static_cast<To>(round(vec.y)));
  }

  template <typename To, typename From, typename RoundFunc>
  auto vector2_round(const Vector2<From>& vec, RoundFunc&& round)
  {
    return make_vector2(static_cast<To>(round(vec.x)), static_cast<To>(round(vec.y)));
  }

  template <typename T>
  Vector2<T> make_vector2(T x, T y)
  {
    return Vector2<T>(x, y);
  }

  template <typename T>
  auto make_3d(Vector2<T> vec, T z = {})
  {
    return Vector3<T>(vec.x, vec.y, z);
  }

  template <typename T>
  auto make_2d(Vector2<T> vec)
  {
    return vec;
  }

  template <typename T>
  std::istream& operator>>(std::istream& stream, Vector2<T>& vec)
  {
    return stream >> vec.x >> vec.y;
  }

  template <typename T>
  std::ostream& operator<<(std::ostream& stream, Vector2<T> vec)
  {
    return stream << vec.x << ", " << vec.y;
  }

  using Vector2u = Vector2<std::uint32_t>;
  using Vector2i = Vector2<std::int32_t>;
  using Vector2f = Vector2<float>;
  using Vector2d = Vector2<double>;
}
