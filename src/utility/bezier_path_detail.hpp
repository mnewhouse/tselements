/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "bezier_path.hpp"

namespace ts
{
  namespace utility
  {
    namespace detail
    {
      template <typename Point>
      BezierPathIterator<Point>::BezierPathIterator(Point* point)
        : point_(point)
      {
      }

      template <typename Point>
      template <typename OtherPoint, typename>
      BezierPathIterator<Point>::BezierPathIterator(const BezierPathIterator<OtherPoint>& other)
        : point_(other.point_)
      {
      }

      template <typename Point>
      bool BezierPathIterator<Point>::equal(const BezierPathIterator& other) const
      {
        return point_ == other.point_;
      }

      template <typename Point>
      void BezierPathIterator<Point>::increment()
      {
        point_ += 3;
      }

      template <typename Point>
      void BezierPathIterator<Point>::decrement()
      {
        point_ -= 3;
      }

      template <typename Point>
      typename BezierPathIterator<Point>::reference
        BezierPathIterator<Point>::dereference() const
      {
        return{ point_[0], point_[1], point_[2], point_[3] };
      }

      template <typename Point>
      void BezierPathIterator<Point>::advance(difference_type d)
      {
        point_ += 3 * d;
      }

      template <typename Point>
      typename BezierPathIterator<Point>::difference_type 
        BezierPathIterator<Point>::distance_to(const BezierPathIterator& other) const
      {
        return (other.point_ - point_) / 3;
      }

      template <typename PointVector, typename ForwardIt>
      void bezier_path_insert(PointVector& vec, std::size_t index, 
                              ForwardIt it, ForwardIt end, std::forward_iterator_tag)
      {
        if (auto point_count = std::distance(it, end) * 3)
        {
          if (vec.empty()) ++point_count;
          else if (index != 0) ++index;

          vec.insert(vec.begin() + index, point_count, {});
          if (index == 0)
          {
            vec.front() = (*it).start_point;
            ++index;
          }
                
          for (auto vec_it = vec.begin() + index; it != end; ++it)
          {
            const auto& curve = *it;

            *vec_it++ = curve.start_control;
            *vec_it++ = curve.end_control;
            *vec_it++ = curve.end_point;
          }
        }
      }

      template <typename PointVector, typename InputIt>
      void bezier_path_insert(PointVector& vec, std::size_t index,
                              InputIt it, InputIt end, std::input_iterator_tag)
      {
        using curve_type = typename std::iterator_traits<InputIt>::value_type;

        std::vector<curve_type> temp_vec(it, end);
        bezier_path_insert(vec, index, temp_vec.begin(), temp_vec.end(), std::random_access_iterator_tag);
      }
    }

    template <typename InputIt>
    BezierPath::BezierPath(InputIt it, InputIt end)
    {
      insert(this->end(), it, end);
    }

    inline std::size_t BezierPath::size() const
    {
      return points_.size() / 3;
    }

    inline bool BezierPath::empty() const
    {
      return points_.empty();
    }

    inline void BezierPath::reserve(std::size_t cap)
    {
      points_.reserve(cap * 3 + 2);
    }

    inline BezierPath::const_iterator BezierPath::begin() const
    {
      return const_iterator(points_.data());
    }

    inline BezierPath::iterator BezierPath::begin()
    {
      return iterator(points_.data());
    }

    inline BezierPath::const_iterator BezierPath::end() const
    {
      return begin() + size();
    }

    inline BezierPath::iterator BezierPath::end()
    {
      return begin() + size();
    }

    template <typename InputIt>
    BezierPath::iterator BezierPath::insert(const_iterator pos, InputIt begin, InputIt end)
    {
      std::size_t index = pos.point_ - points_.data();
      detail::bezier_path_insert(points_, pos.point_ - points_.data(), begin, end,
                                 typename std::iterator_traits<InputIt>::iterator_category());

      return iterator(points_.data() + index);
    }

    inline BezierPath::iterator BezierPath::insert(const_iterator pos, const BezierCurve& curve)
    {
      return insert(pos, &curve, &curve + 1);
    }

    inline void BezierPath::push_back(const BezierCurve& curve)
    {
      insert(end(), curve);
    }

    template <typename CurveType>
    Vector2<double> bezier_point_at(const CurveType& curve, double time_point)
    {
      auto t = time_point;
      auto u = 1.0 - t;
      auto tt = t * t;
      auto uu = u * u;
      auto ttt = tt * t;
      auto uuu = uu * u;

      auto p = uuu * curve.start_point;
      p += 3.0 * uu * t * curve.start_control;
      p += 3.0 * u * tt * curve.end_control;
      p += ttt * curve.end_point;
      return p;
    }

    template <typename CurveType>
    Vector2<double> bezier_normal_at(const CurveType& curve, double time_point)
    {
      auto t = time_point, a = curve.start_point, b = curve.start_control, 
        c = curve.end_control, d = curve.end_point;

      auto c1 = (d - (3.0 * c) + (3.0 * b) - a);
      auto c2 = ((3.0 * c) - (6.0 * b) + (3.0 * a));
      auto c3 = ((3.0 * b) - (3.0 * a));

      return ((3.0 * c1 * time_point * time_point) + (2.0 * c2 * time_point) + c3);
    }
  }
}

#pragma once