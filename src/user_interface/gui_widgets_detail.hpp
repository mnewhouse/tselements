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

    inline WidgetState widget_state(const FloatRect& area, const InputState& input_state)
    {
      auto contained = contains(area, vector2_cast<float>(input_state.mouse_position));

      WidgetState result;
      result.hover_state = contained;

      constexpr auto left_button = static_cast<std::uint32_t>(MouseButton::Left);
      result.click_state = (input_state.mouse_button_state & left_button) != 0 && contained;

      result.was_clicked = contained && (input_state.mouse_button_state & left_button) == 0 &&
        (input_state.old_mouse_button_state & left_button) != 0;

      return result;
    }

    inline auto hover_state(const WidgetState& state)
    {
      return state.hover_state;
    }

    inline auto click_state(const WidgetState& state)
    {
      return state.click_state;
    }

    inline auto was_clicked(const WidgetState& state)
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

    // Empty event processing function to make sure forwarding a parameter pack full of
    // events doesn't error out.
    template <typename WidgetState>
    void process_event(const WidgetState&)
    {
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

      template <typename Area, typename Style, typename Geometry, typename... EventHandlers>
      auto button(const Area& area, const Style& style,
                  const InputState& input_state, Geometry& geometry, EventHandlers&&... event_handlers)
      {
        auto state = widget_state(area, input_state);

        auto button_impl = [&](const auto& style)
        {
          add_background_if_applicable(area, style, geometry);
        };

        do_widget(state, style, button_impl);

        process_event(state, std::forward<EventHandlers>(event_handlers)...);

        return state;
      }
    }
  }
}

#endif