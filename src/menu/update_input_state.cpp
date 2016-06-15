/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "update_input_state.hpp"

namespace ts
{
  namespace menu
  {
    namespace detail
    {
      static auto translate_mouse_button(sf::Mouse::Button button)
      {
        switch (button)
        {
        case sf::Mouse::Left:
          return gui::MouseButton::Left;

        case sf::Mouse::Right:
          return gui::MouseButton::Right;

        case sf::Mouse::Middle:
          return gui::MouseButton::Middle;

        default:
          return gui::MouseButton::None;
        }
      }
    }

    void update_input_state(gui::InputState& input_state, const game::Event& event)
    {
      switch (event.type)
      {
      case sf::Event::MouseButtonPressed:
      {
        auto button = detail::translate_mouse_button(event.mouseButton.button);
        input_state.mouse_button_state |= static_cast<std::uint32_t>(button);
        if (button == gui::MouseButton::Left)
        {
          input_state.click_position.x = event.mouseButton.x;
          input_state.click_position.y = event.mouseButton.y;
        }

        break;
      }

      case sf::Event::MouseButtonReleased:
      {
        auto button = detail::translate_mouse_button(event.mouseButton.button);
        input_state.mouse_button_state &= ~static_cast<std::uint32_t>(button);
        break;
      }

      case sf::Event::MouseMoved:
      {
        input_state.mouse_position.x = event.mouseMove.x;
        input_state.mouse_position.y = event.mouseMove.y;
        break;
      }

      case sf::Event::MouseWheelScrolled:
      {
        input_state.mouse_wheel_delta += event.mouseWheel.delta;
        break;
      }

      default:
        break;
      }
    }

    void update_old_input_state(gui::InputState& input_state)
    {
      input_state.old_mouse_button_state = input_state.mouse_button_state;
      input_state.old_mouse_position = input_state.mouse_position;
    }
  }
}