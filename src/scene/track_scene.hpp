/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resources/geometry.hpp"

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
    struct TrackLayer;
  }

  namespace scene
  {
    class TrackSceneLayer
    {
    public:
      using texture_type = graphics::Texture;
      explicit TrackSceneLayer(std::uint32_t level);

      using vertex_type = resources::Vertex;
      using face_type = resources::Face;

      struct Component
      {
        const texture_type* texture;

        std::uint32_t vertex_index;
        std::uint32_t vertex_count;
        std::uint32_t face_index;
        std::uint32_t face_count;
      };

      const std::vector<Component>& components() const;

      const std::vector<vertex_type>& vertices() const;
      const std::vector<face_type>& faces() const;

      std::uint32_t level() const;

      std::size_t append_item();

      void append_item_geometry(std::size_t item_index, const texture_type* texture,
                                const vertex_type* vertices, std::uint32_t vertex_count,
                                const face_type* faces, std::uint32_t face_count);

      void append_last_item_geometry(const texture_type* texture,
                                     const vertex_type* vertices, std::uint32_t vertex_count,
                                     const face_type* faces, std::uint32_t face_count);

    private:
      struct ItemInfo
      {
        std::uint32_t vertex_index;
        std::uint32_t vertex_count;
        std::uint32_t face_index;
        std::uint32_t face_count;
      };

      std::vector<Component> components_;
      std::vector<ItemInfo> items_;

      std::vector<vertex_type> vertices_; 
      std::vector<face_type> faces_;

      std::uint32_t level_;
    };

    // TrackScene represents the drawable portion of the track.
    // It stores the texture handles, and keeps a list of vertices in the
    // order they must be drawn, grouped by layer.
    class TrackScene
    {
    public:
      using LayerHandle = const resources::TrackLayer*;

      TrackScene() = default;
      explicit TrackScene(Vector2i track_size);

      Vector2i track_size() const;

      using texture_type = TrackSceneLayer::texture_type;
      using vertex_type = TrackSceneLayer::vertex_type;

      const texture_type* register_texture(std::unique_ptr<texture_type> texture);

      TrackSceneLayer* create_layer(LayerHandle track_layer);

      TrackSceneLayer* find_layer(LayerHandle track_layer);
      const TrackSceneLayer* find_layer(LayerHandle track_layer) const;

      using const_layer_range = boost::iterator_range<boost::indirect_iterator<const TrackSceneLayer* const*>>;
      const_layer_range active_layers() const;
      void deactivate_layer(LayerHandle layer_handle);

    private:
      std::vector<std::unique_ptr<texture_type>> textures_;
      std::unordered_map<LayerHandle, TrackSceneLayer> layers_;
      std::vector<TrackSceneLayer*> active_layers_;
      Vector2i track_size_;
    };
  }
}
