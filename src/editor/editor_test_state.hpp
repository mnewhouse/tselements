/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_TEST_STATE_HPP_384192834
#define EDITOR_TEST_STATE_HPP_384192834

#include "game/game_state.hpp"

#include "editor_scene.hpp"

#include "tools/editor_path_tool.hpp"

namespace ts
{
  namespace editor
  {
    class TestState
      : public game::GameState
    {
    public:
      explicit TestState(const game_context& ctx);

      virtual void render(const render_context&) const override;
      virtual void process_event(const event_type& event) override;

    private:
      EditorScene editor_scene_;
      // Tools
      tools::PathTool path_tool_;

      EditorTool* active_tool_ = nullptr;
    };
  }
}

#endif