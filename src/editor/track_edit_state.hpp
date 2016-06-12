/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_EDIT_STATE_HPP_384192834
#define TRACK_EDIT_STATE_HPP_384192834

#include "game/game_state.hpp"

#include "user_interface/gui_geometry.hpp"
#include "user_interface/gui_input_state.hpp"

#include "editor_scene.hpp"

#include "tools/editor_path_tool.hpp"

namespace ts
{
  namespace editor
  {
    class TrackEditState
      : public game::GameState
    {
    public:
      explicit TrackEditState(const game_context& ctx);

      virtual void render(const render_context&) const override;
      virtual void process_event(const event_type& event) override;
      virtual void update(const update_context&) override;

    private:
      EditorScene editor_scene_;
      // Tools
      tools::PathTool path_tool_;

      EditorTool* active_tool_ = nullptr;

      gui::Geometry gui_geometry_;
      gui::InputState input_state_;
    };
  }
}

#endif