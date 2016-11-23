/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_scene.hpp"
#include "editor_tool.hpp"
#include "editor_test_state.hpp"
#include "editor_interface_state.hpp"
#include "editor_working_state.hpp"
#include "editor_action_history.hpp"

#include "tools/editor_tile_tool.hpp"
#include "tools/editor_path_tool.hpp"

#include "game/game_state.hpp"

#include "scene/viewport.hpp"

#include <boost/optional.hpp>

namespace ts
{
  namespace editor
  {
    class EditorState
      : public game::GameState, private InterfaceState
    {
    public:
      explicit EditorState(const game_context& ctx);
      explicit EditorState(const game_context& ctx, const std::string& track_path);

      virtual void render(const render_context&) const override;
      virtual void process_event(const event_type& event) override;
      virtual void update(const update_context&) override;

      virtual void on_activate() override;

    private:
      void async_load_track(const std::string& track_path);
      void async_load_test_state();

      void update_loading_state();

      virtual void active_tool_changed(ToolType previous, ToolType current) override;
      virtual void active_mode_changed(std::uint32_t previous, std::uint32_t current) override;

      void editor_scene_interface();
      void tool_pane_windows();
      void history_window();
      void layers_window();

      template <typename ContextType, typename Self>
      static ContextType make_context(Self&& self);

      EditorContext make_context();
      ImmutableEditorContext make_context() const;

      std::unique_ptr<EditorScene> editor_scene_;
      std::future<std::unique_ptr<EditorScene>> loading_future_;

      std::unique_ptr<TestState> test_state_;
      std::future<std::unique_ptr<TestState>> test_loading_future_;

      scene::Viewport view_port_;

      scene::Camera camera_snapshot_;
      Vector2f canvas_scroll_state_;
      bool canvas_drag_state_ = false;

      EditorTool* active_tool_ = nullptr;
      PathTool path_tool_;
      TileTool tile_tool_;

      WorkingState working_state_;
      ActionHistory action_history_;
    };
  }
}
