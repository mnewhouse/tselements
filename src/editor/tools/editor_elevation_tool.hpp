/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_ELEVATION_TOOL_438198234
#define EDITOR_ELEVATION_TOOL_438198234

#include "editor/editor_tool.hpp"

#include "graphics/buffer.hpp"
#include "graphics/shader.hpp"
#include "graphics/sampler.hpp"

#include "utility/color.hpp"

#include <boost/optional.hpp>

#include <vector>

namespace ts
{
  namespace resources_3d
  {
    class HeightMap;
  }

  namespace editor
  {
    namespace tools
    {
      namespace detail
      {
        enum class ElevationToolShape
        {
          Circular,
          Square
        };

        struct ElevationToolProperties
        {
          ElevationToolShape shape = ElevationToolShape::Square;
          float strength = 0.2f;
          float softness = 0.9f;
          float size = 128.0f;
        };
      }

      class ElevationTool
        : public EditorTool
      {
      public:
        explicit ElevationTool(EditorScene* editor_scene);

        virtual bool update_gui(bool has_focus, const gui::InputState& input_state,
                                gui::Geometry& geometry) override;


        virtual void render() const override;

        virtual void set_active_mode(std::size_t mode) override;

      private:
        void update_interface_buffer(Vector2f hover_position);

        template <typename Function>
        boost::optional<IntRect> apply_height_map_function(Vector2f world_pos, Function function);

        enum Mode
        {
          Heighten,
          Lower,
          Equalize
        };

        using ToolProperties = detail::ElevationToolProperties;

        Mode active_mode_;
        ToolProperties tool_properties_;

        graphics::ShaderProgram interface_shader_;
        graphics::Buffer interface_vertex_buffer_;
        graphics::Buffer interface_index_buffer_;
        graphics::Sampler height_map_sampler_;

        struct MapCoordInterfaceInfo
        {
          Vector2i coords;
          Vector2i position;
          float strength;
        };

        struct InterfaceVertex
        {
          Vector2f position;
          Colorb color;
        };

        std::uint32_t interface_element_count_ = 0;
        std::vector<MapCoordInterfaceInfo> map_coord_interface_info_;
        std::vector<InterfaceVertex> vertex_cache_;
        std::vector<std::uint32_t> index_cache_;

        boost::optional<IntRect> modified_area_;
      };
    }
  }
}

#endif