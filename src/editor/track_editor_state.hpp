/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_EDITOR_STATE_HPP_384192834
#define TRACK_EDITOR_STATE_HPP_384192834

#include "editor_scene.hpp"
#include "track_editor_test_state.hpp"
#include "track_editor_menu.hpp"
#include "track_editor_interface_state.hpp"

#include "game/game_state.hpp"
#include "game/loading_thread.hpp"

#include "scene/viewport.hpp"

#include "user_interface/gui_geometry.hpp"
#include "user_interface/gui_input_state.hpp"

#include "tools/editor_path_tool.hpp"
#include "tools/editor_terrain_tool.hpp"
#include "tools/editor_elevation_tool.hpp"

#include <boost/optional.hpp>

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
        explicit EditorState(const game_context& ctx, const std::string& track_path);

        virtual void render(const render_context&) const override;
        virtual void process_event(const event_type& event) override;
        virtual void update(const update_context&) override;

      private:
        void async_load_track(const std::string& track_path);
        void async_load_test_state();

        void poll_loading_state();

        virtual void active_tool_changed(Tool previous, Tool current) override;
        virtual void active_mode_changed(std::size_t previous, std::size_t current) override;

        std::unique_ptr<EditorScene> editor_scene_;
        std::future<std::unique_ptr<EditorScene>> loading_future_;

        std::unique_ptr<TestState> test_state_;
        std::future<std::unique_ptr<TestState>> test_loading_future_;

        scene::Viewport view_port_;

        Menu track_editor_menu_;

        EditorTool* active_tool_ = nullptr;

        gui::Geometry gui_geometry_;
        gui::InputState input_state_;

        std::vector<event_type> event_queue_;
      };
    }
  }
}

#endif