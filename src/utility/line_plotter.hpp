/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef BRESENHAM_HPP_83495239845
#define BRESENHAM_HPP_83495239845

#include "vector2.hpp"

#include <cmath>
#include <algorithm>

namespace ts
{
  namespace utility
  {
    struct LinePlottingIterator
      : std::iterator<std::forward_iterator_tag, Vector2<double>>
    {
      explicit LinePlottingIterator(Vector2<double> begin, Vector2<double> end)
        : start_value_(begin),
          offset_(end - begin),
          index_(0),          
          step_count_(std::max(static_cast<std::int32_t>(std::abs(offset_.x)),
                               static_cast<std::int32_t>(std::abs(offset_.y))) + 1),      
          step_(1.0 / step_count_)
      {
      }

      struct end_t {};
      explicit LinePlottingIterator(const LinePlottingIterator& other, end_t)
        : start_value_(other.start_value_),
          offset_(other.offset_),
          index_(other.step_count_ + 1),
          step_count_(other.step_count_),
          step_(other.step_)
      {
      }

      std::uint32_t index() const
      {
        return index_;
      }

      Vector2<double> operator*() const
      {
        return start_value_ + offset_ * step_ * static_cast<double>(index_);
      }

      LinePlottingIterator& operator++()
      {
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
      Vector2<double> start_value_;
      Vector2<double> offset_;
      std::uint32_t index_ = 0;
      std::uint32_t step_count_;
      double step_;
    };

    struct LinePlottingRange
    {
    public:
      using iterator = LinePlottingIterator;
      using const_iterator = iterator;

      LinePlottingRange(Vector2<double> begin, Vector2<double> end)
        : begin_(begin, end),
          end_(begin_, iterator::end_t())
      {
      }

      const_iterator begin() const
      {
        return begin_;
      }

      const_iterator end() const
      {
        return end_;
      }

    private:
      iterator begin_, end_;
    };

    inline bool operator==(const LinePlottingIterator& a, const LinePlottingIterator& b)
    {
      return a.index() == b.index();
    }
    
    inline bool operator!=(const LinePlottingIterator& a, const LinePlottingIterator& b)
    {
      return !(a == b);
    }
  }
}

#endif
