/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/rect.hpp"
#include "utility/vector2.hpp"

namespace ts
{
  namespace scene
  {
    class Viewport;
  }

  namespace editor
  {
    class EditorScene;
    class InterfaceState;
    class WorkingState;
    class ActionHistory;

    struct EditorContext
    {
      EditorScene& scene;
      InterfaceState& interface_state;
      WorkingState& working_state;
      ActionHistory& action_history;
      scene::Viewport& canvas_viewport;

      Vector2d screen_size;
      Vector2d world_size;
    };
    
    class CoordTransform
    {
    public:
      explicit CoordTransform(const EditorContext& context);

      Vector2d world_position(Vector2d viewport_pos) const;
      Vector2d viewport_position(Vector2d world_pos) const;

    private:
      DoubleRect screen_rect_;
      Vector2d camera_center_;
      double zoom_;
      double inverse_zoom_;
    };

    Vector2d calculate_world_position(const EditorContext& context, Vector2d viewport_position);
    Vector2d calculate_viewport_position(const EditorContext& context, Vector2d world_position);
  }
}