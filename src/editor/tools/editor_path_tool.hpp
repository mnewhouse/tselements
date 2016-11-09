/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_PATH_TOOL_89358123589
#define EDITOR_PATH_TOOL_89358123589

#include "editor/editor_tool.hpp"

#include "graphics/shader.hpp"
#include "graphics/buffer.hpp"
#include "graphics/sampler.hpp"

#include "editor/track_path.hpp"
#include "editor/model_3d.hpp"

#include "utility/vector3.hpp"
#include "utility/color.hpp"

#include <boost/optional.hpp>

namespace ts
{
  namespace editor
  {
    namespace tools
    {
      class PathTool
        : public EditorTool
      {
      public:
        explicit PathTool(EditorScene* editor_scene);

        virtual void render() const override;

        virtual void process_event(const event_type& event) override;

        virtual bool update_gui(bool has_focus, const gui::InputState& input_state,
                                gui::Geometry& geometry) override;

        virtual void set_active_mode(std::size_t id) override;
        virtual std::size_t active_mode() const override;

      private:
        struct NodeTransformation;
        struct NodePlacement;
        struct SegmentPlacement;
        struct SegmentTransformation;

        void update_path_buffer();

        void update_gui_path_nodes(bool& has_focus, const gui::InputState& input_state,
                                   gui::Geometry& geometry);

        void update_gui_stroke_segments(bool& has_focus, const gui::InputState& input_state,
                                        gui::Geometry& geometry);

        void update_node_transformation(resources_3d::TrackPath& path,
                                        const NodeTransformation& transformation);                                        

        void apply_node_transformation(resources_3d::TrackPath& path,
                                       const NodeTransformation& transformation);

        void update_node_placement(resources_3d::TrackPath& path,
                                   const NodePlacement& node_placement);
        
        void apply_node_placement(resources_3d::TrackPath& path,
                                  const NodePlacement& node_placement);

        void apply_segment_stroke(resources_3d::TrackPath& path,
                                  resources_3d::SegmentedStroke& stroke,
                                  const SegmentPlacement& segment_placement,
                                  std::uint32_t end_index, float end_time);

        void apply_segment_transformation(const resources_3d::TrackPath& path,
                                          resources_3d::SegmentedStroke& stroke,
                                          const SegmentTransformation& transformation);

        struct Vertex
        {
          Vector2f position;
          Colorb color;
        };
     

        enum class Mode
        {
          Nodes,
          StrokeSegments,
        };

        struct NodeTransformation
        {
          Vector2f control_point;
          Vector2f start_point;
          Vector2f end_point;
          std::size_t node_index;

          enum Control
          {
            Node,
            FirstControl,
            SecondControl
          } control;
        };

        struct NodePlacement
        {
          Vector2f end_point;
        };

        enum class SegmentSide
        {
          A,
          B,
          Both
        };

        struct SegmentTransformation
        {
          Vector2f start_point;

          enum SegmentPoint
          {
            Start, End
          } segment_point;

          std::uint32_t segment_index;
        };
        
        struct SegmentPlacement
        {
          Vector2f start_point;
          std::uint32_t start_index;
          float start_time;
          SegmentSide side;          
        };

        graphics::ShaderProgram path_shader_;
        graphics::Buffer path_vertex_buffer_;
        graphics::Buffer path_index_buffer_;
        graphics::Sampler height_map_sampler_;

        std::size_t element_count_ = 0;
        std::size_t line_element_count_ = 0;

        resources_3d::BasicModel<Vertex> path_model_;
        
        Mode active_mode_ = Mode::Nodes;
        boost::optional<std::uint32_t> selected_node_index_;
        boost::optional<NodeTransformation> node_transformation_;
        boost::optional<NodePlacement> node_placement_;

        boost::optional<SegmentTransformation> segment_transformation_;
        boost::optional<SegmentPlacement> segment_placement_;

        boost::optional<std::uint32_t> selected_segment_index_;

        resources_3d::TrackPath* selected_path_ = nullptr;
      };
    }
  }

}

#endif