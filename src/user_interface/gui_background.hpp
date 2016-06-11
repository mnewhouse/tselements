/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_BACKGROUND_HPP_8591889123
#define GUI_BACKGROUND_HPP_8591889123

namespace ts
{
  namespace gui
  {
    namespace detail
    {
      template <typename Area, typename Style, typename Geometry>
      auto add_background_if_applicable(const Area& area, const Style& style,
                                        Geometry& geometry, int)
        -> decltype(add_background(area, style, geometry))
      {
        return add_background(area, style, geometry);
      }

      template <typename Area, typename Style, typename Geometry>
      void add_background_if_applicable(const Area& area, const Style& style,
                                        Geometry& geometry, ...)
      {        
      }
    }

    template <typename Area, typename Style, typename Geometry>
    void add_background_if_applicable(const Area& area, const Style& style, Geometry& geometry)
    {
      detail::add_background_if_applicable(area, style, geometry, 0);
    }
  }
}

#endif