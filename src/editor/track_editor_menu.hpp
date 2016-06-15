/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_EDITOR_MENU_HPP_595901324
#define TRACK_EDITOR_MENU_HPP_595901324

#include "user_interface/gui_input_state.hpp"
#include "user_interface/gui_geometry.hpp"

#include <vector>
#include <cstdint>

namespace ts
{
  namespace fonts
  {
    class FontLibrary;
  }

  namespace editor
  {
    namespace track
    {
      class InterfaceState;

      class Menu
      {
      public:
        explicit Menu(const fonts::FontLibrary* font_library);

        struct UpdateState
        {
          IntRect bounding_box;
          bool focus_state = false;
        };

        UpdateState update(InterfaceState& interface_state, Vector2i screen_size,
                           const gui::InputState& input_state, gui::Geometry& geometry);

      private:
        void show_tools_menu(FloatRect area);
        void show_modes_menu(FloatRect area);

        const fonts::FontLibrary* font_library_;
        std::vector<std::uint32_t> menu_expansion_state_;

        // Make sure we don't have to pass around all the state variables to the sub-menu
        // functions by storing them here.
        InterfaceState* interface_state_;
        gui::Geometry* geometry_;
        const gui::InputState* input_state_;
        Vector2i screen_size_;
      };
    }
  }
}

#endif
