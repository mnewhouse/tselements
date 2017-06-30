/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_scene_3d.hpp"
#include "editor_tool_state_3d.hpp"
#include "editor_path_designer_3d.hpp"
#include "working_state_3d.hpp"

#include "scene_3d/viewport_3d.hpp"

#include "game/game_state.hpp"

#include <memory>

namespace ts
{
  namespace editor3d
  {
    class EditorState
      : public game::GameState
    {
    public:
      explicit EditorState(const game_context& context);

      virtual void render(const render_context& ctx) const override;
      virtual void update(const update_context& ctx) override;
      virtual void process_event(const event_type& event) override;

    private:
      void editor_scene_interface();

      EditorContext make_context();

      std::unique_ptr<EditorScene> editor_scene_;
      scene3d::Viewport view_port_;

      ToolState tool_state_;
      WorkingState working_state_;

      EditorTool* active_tool_ = nullptr;

      // Tools
      PathDesigner path_designer_;
      //TerrainSculptor terrain_sculptor_;
      //TerrainBrush terrain_brush_;
    };
  }
}