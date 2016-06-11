/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_TOOL_HPP_3518945
#define EDITOR_TOOL_HPP_3518945

#include <glm/mat4x4.hpp>

#include "game/game_events.hpp"

namespace ts
{
  namespace editor
  {
    class EditorScene;

    class EditorTool
    {
    public:
      explicit EditorTool(EditorScene* editor_scene)
        : editor_scene_(editor_scene)
      {}

      virtual void update() {}
      virtual void render() const {}

      using event_type = game::Event;
      virtual void process_event(const event_type& event) {}

      EditorScene* editor_scene() { return editor_scene_; }
      const EditorScene* editor_scene() const { return editor_scene_; }

    private:
      EditorScene* editor_scene_;
    };
  }
}

#endif