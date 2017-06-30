/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "editor/editor_tool.hpp"

#include "resources/track_path.hpp"

#include <boost/optional.hpp>

#include <cstddef>

namespace ts
{
  namespace resources
  {
    class TrackLayer;
  }

  namespace editor
  {
    class PathTool
      : public EditorTool
    {
    public:
      virtual void update_tool_info(const EditorContext& context) override;
      virtual void update_canvas_interface(const EditorContext& context) override;

      virtual const char* tool_name() const override;
      virtual mode_name_range mode_names() const override;

      virtual void delete_last(const EditorContext& context) override;
      virtual void delete_selected(const EditorContext& context) override;

    private:
      using Path = resources::TrackPath;
      using Node = resources::TrackPathNode;

      resources::TrackLayer* selected_layer_ = nullptr;
      std::size_t selected_path_index_ = 0;     

      Path working_path_;

      struct NodeTransformation
      {
        enum Action { Append, Move, Insert };
        enum Control { Base, First, Second };

        Node original_state;
        Action action;
        Control control;
        std::uint32_t id;
      };

      void apply_node_transformation(Vector2f world_pos);
      void finalize_node_transformation(const EditorContext& context);

      void update_selection(const WorkingState& working_state);
      void commit_working_path(const EditorScene& scene);
      void reload_working_path();
      void ensure_path_layer_exists(const EditorContext& context);
      
      void select_path(resources::TrackLayer* layer, std::size_t path_index, WorkingState& working_state);

      float path_width_ = 32.0f;
      float path_width_jitter_ = 0.0f;

      boost::optional<NodeTransformation> node_transformation_;
    };
  }
}