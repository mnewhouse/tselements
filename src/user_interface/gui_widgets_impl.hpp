/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_WIDGETS_IMPL_HPP_51139486
#define GUI_WIDGETS_IMPL_HPP_51139486

#include "gui_widgets.hpp"

namespace ts
{
  namespace gui
  {
    namespace widgets 
    {
      template <typename Style, typename OutIt, typename... EventHandlers>
      void button(boost::string_ref text, IntRect rect, const Style& style,
                  const InputState& input_state, OutIt vertex_out, EventHandlers&&... event_handlers)
      {

      }
    }
  }
}

#endif