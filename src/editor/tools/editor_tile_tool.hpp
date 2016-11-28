/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "tile_interaction_renderer.hpp"

#include "editor/editor_tool.hpp"

#include "resources/tiles.hpp"
#include "resources/geometry.hpp"

#include "graphics/texture.hpp"

#include "utility/rotation.hpp"

#include <boost/optional.hpp>

#include <cstdint>

namespace ts
{
  namespace resources
  {
    class TrackLayer;
    class TileLibrary;
  }

  namespace editor
  {
    class TileTool
      : public EditorTool
    {
    public:
      TileTool();

      virtual void update_tool_info(const EditorContext& context) override;
      virtual void update_canvas_interface(const EditorContext& context) override;

      virtual void delete_last(const EditorContext& context) override;
      virtual void delete_selected(const EditorContext& context) override;

      virtual void next(const EditorContext& context) override;
      virtual void previous(const EditorContext& context) override;

      virtual void activate(const EditorContext& context) override;
      virtual void deactivate(const EditorContext& context) override;

      virtual void on_canvas_render(const ImmutableEditorContext& context, const glm::mat4& matrix) const override;

      virtual const char* tool_name() const override;
      virtual mode_name_range mode_names() const override;

    private:
      void set_placement_tile_id(std::uint32_t tile_id, const resources::TileLibrary& tile_libray);
      void update_placement_tile_preview(EditorScene& editor_scene, Vector2d world_pos);
      void update_selected_layer(const EditorContext& context);

      void reload_tile_library_cache(const EditorScene& editor_scene);

      boost::optional<std::uint32_t> placement_tile_id() const;
      void place_tile_at(EditorScene& scene, Vector2d world_pos);

      resources::TrackLayer* selected_layer_ = nullptr;

      const resources::TileDefinition* placement_tile_ = nullptr;
      const resources::TileGroupDefinition* placement_tile_group_ = nullptr;

      bool placement_tile_dirty_ = false;
      bool scroll_to_selected_ = false;

      std::int32_t selected_tile_category_ = 0;
      std::int32_t placement_tile_rotation_ = 0;
      
      struct TileQuad
      {
        std::uint32_t tile_id;
        void* texture_handle;
        std::array<resources::Vertex, 4> vertices;        
      };

      Vector2f tile_library_cell_size_ = { 60.0f, 60.0f };
      std::vector<TileQuad> tile_library_quad_cache_;
      std::vector<resources::PlacedTile> tile_expansion_cache_;
      graphics::Texture tiled_transparency_texture_;

      TileInteractionRenderer tile_interaction_renderer_;      
    };
  }
}
