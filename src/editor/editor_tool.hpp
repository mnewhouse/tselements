/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_TOOL_HPP_3518945
#define EDITOR_TOOL_HPP_3518945

#include "game/game_events.hpp"

#include "utility/vector2.hpp"
#include "utility/rect.hpp"

#include "user_interface/gui_mouse.hpp"

#include <glm/mat4x4.hpp>

namespace ts
{
  namespace gui
  {
    struct Geometry;
    struct InputState;
  }

  namespace editor
  {
    class EditorScene;

    class EditorTool
    {
    public:
      explicit EditorTool(EditorScene* editor_scene)
        : editor_scene_(editor_scene)
      {}

      virtual void render() const {}

      virtual void set_active_mode(std::size_t mode_id) {};
      virtual std::size_t active_mode() const { return 0; }

      using event_type = game::Event;
      virtual void process_event(const event_type& event) {}

      EditorScene* editor_scene() { return editor_scene_; }
      const EditorScene* editor_scene() const { return editor_scene_; }

      virtual bool update_gui(bool has_focus, const gui::InputState& input_state,
                              gui::Geometry& geometry) 
      {
        return has_focus;
      }

    private:
      EditorScene* editor_scene_;
    };
  }
}

#endif