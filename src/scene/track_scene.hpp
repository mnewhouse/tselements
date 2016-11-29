/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "texture_mapping.hpp"

#include "resources/geometry.hpp"
#include "resources/tiles.hpp"

#include "graphics/texture.hpp"

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/iterator_range.hpp>

#include <vector>
#include <memory>
#include <unordered_map>

namespace ts
{
  namespace resources
  {
    class TrackLayer;
  }

  namespace scene
  {
    class TrackSceneLayer
    {
    public:
      using texture_type = graphics::Texture;
      using vertex_type = resources::Vertex;
      using face_type = resources::Face;

      explicit TrackSceneLayer(const resources::TrackLayer* associated_layer);

      struct Component
      {
        std::uint32_t level;
        const texture_type* texture;

        std::uint32_t face_index;
        std::uint32_t face_count;
      };

      struct GeometryUpdate
      {
        std::uint32_t vertex_begin;
        std::uint32_t vertex_end;
        std::uint32_t face_begin;
        std::uint32_t face_end;
      };

      std::uint32_t base_level() const;

      const std::vector<Component>& components() const;
      const std::vector<vertex_type>& vertices() const;
      const std::vector<face_type>& faces() const;

      GeometryUpdate append_item_geometry(std::uint32_t item_index, const texture_type* texture,
                                          const vertex_type* vertices, std::uint32_t vertex_count,
                                          const face_type* faces, std::uint32_t face_count,
                                          std::uint32_t level);

      GeometryUpdate remove_item_geometry(std::uint32_t item_index, bool shift = true);
      
      bool visible() const;
      void hide();
      void show();

      const resources::TrackLayer* associated_layer() const;

    private:
      struct ItemInfo
      {
        std::uint32_t item_index;
        std::uint32_t level;
        std::uint32_t vertex_index;
        std::uint32_t vertex_count;
        std::uint32_t face_index;
        std::uint32_t face_count;
      };

      std::vector<Component> components_;
      std::vector<ItemInfo> items_;

      std::vector<vertex_type> vertices_; 
      std::vector<face_type> faces_;

      const resources::TrackLayer* associated_layer_;

      std::uint32_t base_level_;
      bool is_visible_ = true;
    };

    // TrackScene represents the drawable portion of the track.
    // It stores the texture handles, and keeps a list of vertices in the
    // order they must be drawn, grouped by layer.
    class TrackScene
    {
    public:
      using LayerHandle = const resources::TrackLayer*;
      explicit TrackScene(Vector2i track_size, TextureMapping texture_mapping);

      Vector2i track_size() const;

      using texture_type = TrackSceneLayer::texture_type;
      using vertex_type = TrackSceneLayer::vertex_type;
      
      void create_layer(LayerHandle layer_handle);      
      void deactivate_layer(LayerHandle layer_handle);

      const TextureMapping& texture_mapping() const;

      using GeometryUpdate = TrackSceneLayer::GeometryUpdate;

      GeometryUpdate add_tile_geometry(const resources::TrackLayer* layer, std::uint32_t tile_index,
                                       const resources::PlacedTile* expanded_tile, std::size_t count);

      GeometryUpdate remove_item_geometry(const resources::TrackLayer* layer, 
                                          std::uint32_t item_index);
      
      using layer_range = boost::iterator_range<boost::indirect_iterator<const TrackSceneLayer* const*>>;
      layer_range layers() const;

      const TrackSceneLayer* find_layer(LayerHandle) const;      

      struct Component
      {
        const TrackSceneLayer* scene_layer;
        const resources::TrackLayer* track_layer;
        const graphics::Texture* texture;
        std::uint32_t face_index;
        std::uint32_t face_count;
        std::uint32_t level;
      };

      const std::vector<Component>& components() const;
      const std::vector<Component>& reload_components();
      const std::vector<Component>& sort_components();

    private:
      TrackSceneLayer* find_layer_internal(LayerHandle);

      std::unordered_map<LayerHandle, TrackSceneLayer> layer_map_;
      std::vector<TrackSceneLayer*> layer_list_;
      std::vector<Component> components_;
      std::vector<resources::Vertex> vertex_cache_;
      std::vector<resources::Face> face_cache_;

      TextureMapping texture_mapping_;
      Vector2i track_size_;
    };
  }
}
