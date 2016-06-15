/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_EDIT_STATE_HPP_384192834
#define TRACK_EDIT_STATE_HPP_384192834

#include "editor_scene.hpp"
#include "track_editor_menu.hpp"
#include "track_editor_interface_state.hpp"

#include "game/game_state.hpp"

#include "user_interface/gui_geometry.hpp"
#include "user_interface/gui_input_state.hpp"

#include "tools/editor_path_tool.hpp"

namespace ts
{
  namespace editor
  {
    namespace track
    {
      class EditorState
        : public game::GameState, public InterfaceState
      {
      public:
        explicit EditorState(const game_context& ctx);

        virtual void render(const render_context&) const override;
        virtual void process_event(const event_type& event) override;
        virtual void update(const update_context&) override;

      private:
        virtual void active_tool_changed(Tool previous, Tool current) override;
        virtual void active_mode_changed(std::size_t previous, std::size_t current) override;

        EditorScene editor_scene_;
        Menu track_editor_menu_;
        // Tools
        tools::PathTool path_tool_;

        EditorTool* active_tool_ = nullptr;

        gui::Geometry gui_geometry_;
        gui::InputState input_state_;

        std::vector<event_type> event_queue_;
      };
    }
  }
}

#endif