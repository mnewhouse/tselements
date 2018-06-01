/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor_tool_3d.hpp"

#include "resources_3d/track_path_3d.hpp"
#include "resources_3d/path_vertices_3d.hpp"

#include <boost/optional.hpp>

#include <cstdint>

namespace ts
{
  namespace editor3d
  {
    struct WorkingState;

    class PathDesigner
      : public EditorTool
    {
    public:
      virtual void update_canvas_interface(const EditorContext& ctx) override;

    private:
      void apply_node_transformation(Vector2f world_pos);
      void finalize_node_transformation(const EditorContext& context);

      void update_selection(const WorkingState& working_state);
      void ensure_path_layer_exists(const EditorContext& context);

      void reload_working_path();

      using Path = resources3d::Path;
      using Node = resources3d::PathNode;

      resources3d::PathLayer* selected_path_ = nullptr;
      std::uint32_t selected_path_index_ = 0;

      Path working_path_;
      float current_path_width_ = 56.0f;

      struct NodeTransformation
      {
        enum Action { Append, Move, Insert };
        enum Control { Base, First, Second };

        Node original_state;
        Action action;
        Control control;
        std::uint32_t id;
      };

      boost::optional<NodeTransformation> node_transformation_;

      std::vector<resources3d::EdgeIntersection> edge_intersections_;
      std::vector<resources3d::CellCorner> contained_cell_corners_;
    };
  }
}