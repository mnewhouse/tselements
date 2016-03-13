/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef BRESENHAM_HPP_83495239845
#define BRESENHAM_HPP_83495239845

#include "vector2.hpp"

#include <iterator>
#include <cmath>

namespace ts
{
  namespace utility
  {
    class LinePlottingRange;

    struct LinePlottingIterator
      : std::iterator<std::forward_iterator_tag, Vector2i>
    {
    public:
      LinePlottingIterator(Vector2i start, Vector2i end)
        : vx_(start.x),
          vy_(start.y),
          ix_(start.x < end.x ? 1 : -1),
          iy_(start.y < end.y ? 1 : -1),
          dx_(std::abs(start.x - end.x)), 
          dy_(std::abs(start.y - end.y)),
          err_(dx_ - dy_),
          index_(0)
      {
      }

      struct end_tag {};
      LinePlottingIterator(end_tag, std::uint32_t index)
        : index_(index)
      {
      }

      Vector2i operator*() const
      {
        return{ vx_, vy_ };
      }
      
      LinePlottingIterator& operator++()
      {
        auto e2 = err_ * 2;
        if (e2 >= -dy_)
        {
          err_ -= dy_;
          vx_ += ix_;
        }

        if (e2 < dx_)
        {
          err_ += dx_;
          vy_ += iy_;
        }

        ++index_;
        return *this;
      }

      LinePlottingIterator operator++(int)
      {
        auto temp = *this;
        ++*this;
        return temp;
      }

    private:
      friend LinePlottingRange;
      friend inline bool operator==(const LinePlottingIterator& a, const LinePlottingIterator& b)
      {
        return a.index_ == b.index_;
      }

      friend inline bool operator!=(const LinePlottingIterator& a, const LinePlottingIterator& b)
      {
        return !(a == b);
      }

      std::int32_t vx_, vy_, ix_, iy_, dx_, dy_, err_;
      std::uint32_t index_;
    };

    class LinePlottingRange
    {
    public:
      using iterator = LinePlottingIterator;
      using const_iterator = iterator;

      LinePlottingRange(Vector2i begin, Vector2i end)
        : begin_(begin, end),
          end_(iterator::end_tag(), std::max(begin_.dx_, begin_.dy_) + 1)
      {
      }

      auto begin() const
      {
        return begin_;
      }

      auto end() const
      {
        return end_;
      }

      auto step_count() const
      {
        return end_.index_;
      }

    private:
      iterator begin_, end_;
    };
  }
}

#endif
