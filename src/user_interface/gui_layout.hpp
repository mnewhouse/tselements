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
        FloatRect operator()(FloatRect area) const
        {
          area.left += area.width;
          return area;
        }
      };

      struct GridIncrement
      {
        FloatRect operator()(FloatRect area) const
        {
          area.left += area.width;
          if (area.left >= max_x)
          {
            area.top += area.height;
            area.left = 0.0f;
          }

          return area;
        }

        float max_x;
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

    inline auto grid_layout_generator(Vector2f initial_position, Vector2f area_size, float max_width)
    {
      auto max_x = initial_position.x + max_width;

      return LayoutGenerator<detail::GridIncrement>(FloatRect(initial_position, area_size),
                                                    detail::GridIncrement{ max_x });
    
    }
  }
}

#endif