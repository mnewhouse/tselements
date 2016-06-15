/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_PATH_TOOL_89358123589
#define EDITOR_PATH_TOOL_89358123589

#include "editor/editor_tool.hpp"

#include "graphics/shader.hpp"
#include "graphics/vertex_buffer.hpp"
#include "graphics/sampler.hpp"

#include "editor/track_path.hpp"

#include "utility/vector3.hpp"

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

      private:
        void update_path_buffer();

        graphics::ShaderProgram path_shader_;
        graphics::Buffer path_vertex_buffer_;
        graphics::Buffer path_index_buffer_;
        graphics::Sampler height_map_sampler_;

        resources_3d::TrackPath* selected_path_ = nullptr;
        boost::optional<Vector2i> click_position_;
        std::size_t element_count_ = 0;
        std::size_t line_element_count_ = 0;
        
        enum class Mode
        {
          Nodes
        };
      };
    }
  }

}

#endif