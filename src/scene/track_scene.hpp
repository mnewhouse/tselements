/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "texture_mapping.hpp"
#include "path_geometry.hpp"

#include "resources/geometry.hpp"
#include "resources/tiles.hpp"

#include "graphics/texture.hpp"

#include <boost/optional.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/container/small_vector.hpp>

#include <vector>
#include <memory>
#include <map>

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

      enum Type
      {
        BaseTerrain,
        Default,
        Path,
        PathWithTransparency
      };

      explicit TrackSceneLayer(const resources::TrackLayer* associated_layer, std::uint32_t level_offset);

      struct Component
      {
        const texture_type* texture;
        std::vector<face_type> faces;
        IntRect bounding_box;
      };

      std::uint32_t level() const;
      std::uint32_t z_index() const;

      using ComponentContainer = std::vector<boost::container::small_vector<Component, 1>>;

      const ComponentContainer& component_regions() const;
      const std::vector<vertex_type>& vertices() const;

      void append_geometry(const texture_type* texture,
                           const vertex_type* vertices, std::uint32_t vertex_count,
                           const face_type* faces, std::uint32_t face_count);
      
      bool visible() const;
      void hide();
      void show();
      void clear();
      Type type() const;
      void set_type(Type type);

      const resources::TrackLayer* associated_layer() const;

      void store_texture(std::unique_ptr<graphics::Texture> tex);
      
      void set_primary_texture(const texture_type* tex);
      void set_secondary_texture(const texture_type* tex);

      const texture_type* primary_texture() const;
      const texture_type* secondary_texture() const;

      void set_primary_texture_tile_size(Vector2f tile_size);
      void set_secondary_texture_tile_size(Vector2f tile_size);

      Vector2f primary_texture_tile_size() const;
      Vector2f secondary_texture_tile_size() const;

    private:
      ComponentContainer components_;
      Type type_ = Type::Default;

      const resources::TrackLayer* associated_layer_;
      std::vector<vertex_type> vertices_;

      std::unique_ptr<graphics::Texture> stored_texture_;
      const texture_type* primary_texture_ = nullptr;
      const texture_type* secondary_texture_ = nullptr;

      Vector2f primary_texture_tile_size_ = { 256.0f, 256.0f };
      Vector2f secondary_texture_tile_size_ = { 256.0f, 256.0f };

      std::uint32_t level_offset_ = 0;
      bool is_visible_ = true;
    };

    // TrackScene represents the drawable portion of the track.
    // It stores the texture handles, and keeps a list of vertices in the
    // order they must be drawn, grouped by layer.
    class TrackScene
    {
    public:
      TrackScene() = default;

      using LayerHandle = const resources::TrackLayer*;
      explicit TrackScene(Vector2i track_size, TextureMapping texture_mapping);

      Vector2i track_size() const;

      using texture_type = TrackSceneLayer::texture_type;
      using vertex_type = TrackSceneLayer::vertex_type;
      
      TrackSceneLayer& scene_layer(LayerHandle layer_handle, std::uint32_t level = 0);
      void deactivate_layer(LayerHandle layer_handle);

      const TextureMapping& texture_mapping() const;

      void add_tile_geometry(const resources::TrackLayer* layer,
                             const resources::PlacedTile* expanded_tile, std::size_t count);
      void rebuild_tile_layer_geometry(const resources::TrackLayer* layer,
                                       const resources::PlacedTile* expanded_tile, std::size_t count);

      void add_base_terrain_geometry(const resources::TrackLayer* layer);
      void rebuild_path_layer_geometry(const resources::TrackLayer* layer);
      
      using layer_range = boost::iterator_range<boost::indirect_iterator<const TrackSceneLayer* const*>>;
      layer_range layers() const;

      const TrackSceneLayer* find_layer(const resources::TrackLayer* layer, std::uint32_t level) const;      

    private:
      TrackSceneLayer* find_layer_internal(LayerHandle, std::uint32_t level = 0);      

      std::map<std::pair<LayerHandle, std::uint32_t>, TrackSceneLayer> layer_map_;
      std::vector<TrackSceneLayer*> layer_list_;
      std::vector<resources::Vertex> vertex_cache_;
      std::vector<resources::Face> face_cache_;

      TextureMapping texture_mapping_;
      Vector2i track_size_;
    };
  }
}
