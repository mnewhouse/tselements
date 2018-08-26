/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_scene.hpp"
#include "editor_mode.hpp"
#include "editor_test_state.hpp"
#include "editor_interface_state.hpp"
#include "editor_working_state.hpp"
#include "editor_action_history.hpp"

#include "modes/editor_tile_mode.hpp"
#include "modes/editor_path_mode.hpp"
#include "modes/editor_control_points_mode.hpp"

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
      void select_default_layer();

      virtual void active_mode_changed(ModeType previous, ModeType current) override;
      virtual void active_tool_changed(std::uint32_t previous, std::uint32_t current) override;

      void editor_scene_interface();
      void tool_pane_windows();
      void history_window();
      void layers_window();

      void deactivate_layer(resources::TrackLayer* layer);
      void toggle_layer_visibility(resources::TrackLayer* layer);
      void open_create_layer_dialog();
      void show_layer_properties(resources::TrackLayer* layer);

      template <typename ContextType, typename Self>
      static ContextType make_context(Self&& self);

      EditorContext make_context();
      ImmutableEditorContext make_context() const;

      std::unique_ptr<EditorScene> editor_scene_;
      std::future<std::unique_ptr<EditorScene>> loading_future_;

      std::future<StageComponents> test_loading_future_;

      scene::Viewport view_port_;

      scene::Camera camera_snapshot_;
      Vector2f canvas_scroll_state_;
      bool canvas_hover_state_ = false;
      bool canvas_focus_state_ = false;
      bool canvas_drag_state_ = false;
      bool layer_rename_window_open_ = false;

      resources::TrackLayer* modified_layer_ = nullptr;
      

      EditorMode* active_mode_ = nullptr;
      PathMode path_mode_;
      TileMode tile_mode_;
      ControlPointsMode control_points_mode_;

      WorkingState working_state_;
      ActionHistory action_history_;
    };
  }
}
