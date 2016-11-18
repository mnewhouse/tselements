/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "vector2.hpp"

#include <type_traits>
#include <algorithm>
#include <iosfwd>

namespace ts
{
  template <typename T>
  struct Rect
  {
  public:
    Rect()
      : left(), top(), width(), height()
    {}

    template <typename T1, typename T2>
    Rect(Vector2<T1> point, Vector2<T2> size)
      : left(point.x), top(point.y), width(size.x), height(size.y)
    {}

    Rect(T left, T top, T width, T height)
      : left(left), top(top), width(width), height(height)
    {}

    T right() const { return left + width; }
    T bottom() const { return top + height; }

    T left;
    T top;
    T width;
    T height;
  };

  template <typename T, typename U>
  auto translate(Rect<T> a, Vector2<U> b)
  {
    a.left += b.x;
    a.top += b.y;
    return a;
  }

  template <typename T, typename U>
  auto combine(Rect<T> a, Rect<U> b)
  {
    using result_type = typename std::common_type<T, U>::type;

    auto min_x = std::min<result_type>(a.left, b.left);
    auto max_x = std::max<result_type>(a.right(), b.right());

    auto min_y = std::min<result_type>(a.top, b.top);
    auto max_y = std::max<result_type>(a.bottom(), b.bottom());

    auto width = max_x - min_x;
    auto height = max_y - min_y;

    return Rect<result_type>(min_x, min_y, width, height);
  }

  template <typename T, typename U>
  auto combine(Rect<T> a, Vector2<U> p)
  {
    using result_type = typename std::common_type<T, U>::type;

    auto min_x = std::min<result_type>(a.left, p.x);
    auto max_x = std::max<result_type>(a.right(), p.x);

    auto min_y = std::min<result_type>(a.top, p.y);
    auto max_y = std::max<result_type>(a.bottom(), p.y);

    auto width = max_x - min_x;
    auto height = max_y - min_y;

    return Rect<result_type>(min_x, min_y, width, height);
  }

  template <typename T>
  bool contains(const Rect<T>& rect, const Vector2<T>& point)
  {
    return point.x >= rect.left && point.y >= rect.top &&
      point.x < rect.right() && point.y < rect.bottom();
  }

  template <typename T>
  bool contains_inclusive(const Rect<T>& rect, const Vector2<T>& point)
  {
    return point.x >= rect.left && point.y >= rect.top &&
      point.x <= rect.right() && point.y <= rect.bottom();
  }

  template <typename T>
  bool contains(const Rect<T>& rect, const Rect<T>& other)
  {
    return rect.left <= other.left && rect.top <= other.top &&
      rect.right() >= other.right() && rect.bottom() >= other.bottom();
  }

  template <typename T>
  bool intersects(const Rect<T>& a, const Rect<T>& b)
  {
    return a.left < b.right() && a.top < b.bottom() &&
      b.left < a.right() && b.top < a.bottom();
  }

  template <typename T>
  auto make_rect_from_points(const Vector2<T>& a, const Vector2<T>& b)
  {
    auto x = std::minmax(a.x, b.x);
    auto y = std::minmax(a.y, b.y);
    return Rect<T>(x.first, y.first, x.second - x.first, y.second - y.first);
  }

  template <typename T>
  auto make_rect_from_points(std::initializer_list<Vector2<T>> points)
  {
    auto x = std::minmax_element(points.begin(), points.end(),
                                 [](const Vector2<T>& a, const Vector2<T>& b)
    {
      return a.x < b.x;
    });

    auto y = std::minmax_element(points.begin(), points.end(),
                                 [](const Vector2<T>& a, const Vector2<T>& b)
    {
      return a.y < b.y;
    });

    return Rect<T>(x.first->x, y.first->y, x.second->x - x.first->x, y.second->y - y.first->y);
  }


  template <typename T, typename U>
  Rect<typename std::common_type<T, U>::type> intersection(const Rect<T>& a, const Rect<U>& b)
  {
    Rect<typename std::common_type<T, U>::type> result;
    result.left = std::max(a.left, b.left);
    result.top = std::max(a.top, b.top);

    result.width = std::min(a.right(), b.right()) - result.left;
    result.height = std::min(a.bottom(), b.bottom()) - result.top;

    return result;
  }

  template <typename T>
  auto size(const Rect<T>& rect)
  {
    return make_vector2(rect.width, rect.height);
  }

  template <typename T>
  auto position(const Rect<T>& rect)
  {
    return make_vector2(rect.left, rect.top);
  }

  template <typename To, typename From>
  Rect<To> rect_cast(const Rect<From>& rect)
  {
    return Rect<To>(static_cast<To>(rect.left), static_cast<To>(rect.top),
                    static_cast<To>(rect.width), static_cast<To>(rect.height));
  }

  template <typename T>
  bool operator==(const Rect<T>& lhs, const Rect<T>& rhs)
  {
    return lhs.left == rhs.left && lhs.top == rhs.top && lhs.width == rhs.width && lhs.height == rhs.height;
  }

  template <typename T>
  bool operator!=(const Rect<T>& lhs, const Rect<T>& rhs)
  {
    return !(lhs == rhs);
  }

  template <typename T>
  std::istream& operator>>(std::istream& stream, Rect<T>& rect)
  {
    return stream >> rect.left >> rect.top >> rect.width >> rect.height;
  }

  template <typename T>
  std::ostream& operator<<(std::ostream& stream, const Rect<T>& rect)
  {
    return stream << rect.left << ", " << rect.top << ", " << rect.width << ", " << rect.height;
  }

  using FloatRect = Rect<float>;
  using DoubleRect = Rect<double>;
  using IntRect = Rect<std::int32_t>;
}
