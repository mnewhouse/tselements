/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace scene3d
  {
    class Viewport;
  }

  namespace editor3d
  {
    class EditorScene;
    struct ToolState;    
    struct WorkingState;
    
    struct EditorContext
    {
      EditorScene& editor_scene;
      ToolState& tool_state;
      WorkingState& working_state;
      scene3d::Viewport& view_port;     
    };
  }
}