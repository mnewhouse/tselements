/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_LAYOUT_HPP_58923846
#define GUI_LAYOUT_HPP_58923846

#include "utility/rect.hpp"

namespace ts
{
  namespace gui
  {
    namespace detail
    {
      struct VerticalIncrement
      {
        FloatRect operator()(FloatRect area) const
        {
          area.top += area.height;
          return area;
        }
      };

      struct HorizontalIncrement
      {
        FloatRect operator()(FloatRect area, Vector2f size) const
        {
          area.left += area.width;
          return area;
        }
      };
    }

    template <typename Increment>
    class LayoutGenerator
    {
    public:
      explicit LayoutGenerator(FloatRect area, Increment increment = {})
        : area_(area),
          increment_(increment)
      {       
      }

      FloatRect operator()(Vector2f area_size)
      {
        auto area = area_;
        area.width = area_size.x;
        area.height = area_size.y;

        area_ = increment(area);
        return area;
      }

      FloatRect operator()()
      {
        auto area = area_;
        area_ = increment_(area);
        return area;
      }

    private:
      FloatRect area_;
      Increment increment_;
    };

    inline auto vertical_layout_generator(Vector2f initial_position, Vector2f area_size = {})
    {
      return LayoutGenerator<detail::VerticalIncrement>(FloatRect(initial_position, area_size));
    }

    inline auto horizontal_layout_generator(Vector2f initial_position, Vector2f area_size)
    {
      return LayoutGenerator<detail::HorizontalIncrement>(FloatRect(initial_position, area_size));
    }
  }
}

#endif