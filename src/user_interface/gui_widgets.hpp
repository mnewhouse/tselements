/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_WIDGETS_HPP_2395481823
#define GUI_WIDGETS_HPP_2395481823

#include <boost/utility/string_ref.hpp>

#include "utility/rect.hpp"

namespace ts
{
  namespace gui
  {
    struct InputState;

    namespace widgets
    {
      template <typename Area, typename Style, typename Geometry, typename... EventHandlers>
      auto button(boost::string_ref text, const Area& area, const Style& style,
                  const InputState& input_state, Geometry& vertex_cache, 
                  EventHandlers&&... event_handlers);
    }
  }
}

#include "gui_widgets_impl.hpp"

#endif