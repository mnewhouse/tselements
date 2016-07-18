/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "editor_terrain_tool.hpp"

namespace ts
{
  namespace editor
  {
    namespace tools
    {
      TerrainTool::TerrainTool(EditorScene* editor_scene)
        : EditorTool(editor_scene)
      {
      }

      bool TerrainTool::update_gui(bool has_focus, const gui::InputState& input_state,
                                   gui::Geometry& geometry)
      {
        return has_focus;
      }

      void TerrainTool::process_event(const event_type& event)
      {

      }
    }
  }
}