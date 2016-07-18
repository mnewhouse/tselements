/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_TERRAIN_TOOL_2859235349
#define EDITOR_TERRAIN_TOOL_2859235349

#include "editor/editor_tool.hpp"

namespace ts
{
  namespace editor
  {
    namespace tools
    {
      class TerrainTool
        : public EditorTool
      {
      public:
        explicit TerrainTool(EditorScene* editor_scene);

        virtual void process_event(const event_type& event) override;

        virtual bool update_gui(bool has_focus, const gui::InputState& input_state,
                                gui::Geometry& geometry) override;

      private:
        
      };
    }
  }
}
#endif