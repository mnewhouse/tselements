/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_WIDGETS_IMPL_HPP_51139486
#define GUI_WIDGETS_IMPL_HPP_51139486

#include "gui_widgets.hpp"
#include "gui_background.hpp"
#include "gui_input_state.hpp"

namespace ts
{
  namespace gui
  {
    struct WidgetState
    {
      bool hover_state = false;
      bool click_state = false;
      bool was_clicked = false;
    };

    WidgetState widget_state(const FloatRect& area, const InputState& input_state)
    {
      auto contained = contains(area, vector2_cast<float>(input_state.click_position));

      WidgetState result;
      result.hover_state = contains(area, vector2_cast<float>(input_state.mouse_position));      

      result.click_state = (input_state.mouse_button_state & mouse_button::left) != 0 && contained;

      result.was_clicked = contained && (input_state.mouse_button_state & mouse_button::left) == 0 &&
        (input_state.old_mouse_button_state & mouse_button::left) != 0;

      return result;
    }

    auto hover_state(const WidgetState& state)
    {
      return state.hover_state;
    }

    auto click_state(const WidgetState& state)
    {
      return state.click_state;
    }

    auto was_clicked(const WidgetState& state)
    {
      return state.was_clicked;
    }

    template <typename Style, typename WidgetFunc>
    void do_widget(const WidgetState& state, const Style& style, WidgetFunc&& func)
    {
      if (state.click_state) func(click_style(style));
      else if (state.hover_state) func(hover_style(style));
      else func(style);
    }

    namespace widgets 
    {
      template <typename Area, typename Style, typename Geometry, typename... EventHandlers>
      auto button(boost::string_ref text, const Area& area, const Style& style,
                  const InputState& input_state, Geometry& geometry, EventHandlers&&... event_handlers)
      {
        auto state = widget_state(area, input_state);

        auto button_impl = [&](const auto& style)
        {
          add_background_if_applicable(area, style, geometry);
          add_text(text, area, style, geometry);
        };

        do_widget(state, style, button_impl);

        process_event(state, std::forward<EventHandlers>(event_handlers)...);

        return state;
      }
    }
  }
}

#endif