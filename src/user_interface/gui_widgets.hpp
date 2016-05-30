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
      template <typename Style, typename OutIt, typename... EventHandlers>
      void button(boost::string_ref text, IntRect rect, const Style& style,
                  const InputState& input_state, OutIt vertex_out, EventHandlers&&... event_handlers);
    }
  }
}

#endif