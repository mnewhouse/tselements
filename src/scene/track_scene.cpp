/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "track_scene.hpp"

#include "resources/track_layer.hpp"

#include <algorithm>

namespace ts
{
  namespace scene
  {
    namespace detail
    {
      template <typename Components>
      auto find_component(Components& components, std::uint32_t vertex_index)
      {
        using std::begin;
        using std::end;

        // Find the matching component
        return std::lower_bound(begin(components), end(components), vertex_index,
                                [](const auto& component, std::uint32_t vertex_index)
        {
          return component.vertex_index + component.vertex_count < vertex_index;
        });
      }
    }
    

    TrackScene::TrackScene(Vector2i track_size, TextureMapping texture_mapping)
      : texture_mapping_(std::move(texture_mapping)),
        track_size_(track_size)  
    {
    }

    const TextureMapping& TrackScene::texture_mapping() const
    {
      return texture_mapping_;
    }

    Vector2i TrackScene::track_size() const
    {
      return track_size_;
    }

    TrackSceneLayer* TrackScene::create_layer(LayerHandle track_layer)
    {
      auto result = layers_.insert(std::make_pair(track_layer, TrackSceneLayer(track_layer->level())));
      if (result.second)
      {
        active_layers_.push_back(&result.first->second);
        return active_layers_.back();
      }

      return nullptr;
    }

    TrackScene::const_layer_range TrackScene::active_layers() const
    {
      return const_layer_range(active_layers_.data(),
                               active_layers_.data() + active_layers_.size());
    }

    TrackSceneLayer* TrackScene::find_layer(LayerHandle track_layer)
    {
      auto it = layers_.find(track_layer);
      if (it != layers_.end()) return &it->second;
      
      return nullptr;
    }

    const TrackSceneLayer* TrackScene::find_layer(LayerHandle track_layer) const
    {
      auto it = layers_.find(track_layer);
      if (it != layers_.end()) return &it->second;

      return nullptr;
    }

    std::size_t TrackSceneLayer::append_item()
    {
      ItemInfo item_info;
      item_info.face_count = 0;
      item_info.vertex_count = 0;

      auto item_index = items_.size();
      if (item_index != 0)
      {
        const auto& last = items_.back();
        item_info.vertex_index = last.vertex_index + last.vertex_count;
        item_info.face_index = last.face_index + last.face_count;        
      }

      items_.push_back(item_info);
      return item_index;
    }

    TrackSceneLayer::TrackSceneLayer(std::uint32_t level)
      : level_(level)
    {
    }

    const std::vector<resources::Face>& TrackSceneLayer::faces() const
    {
      return faces_;
    }

    const std::vector<resources::Vertex>& TrackSceneLayer::vertices() const
    {
      return vertices_;
    }

    const std::vector<TrackSceneLayer::Component>& TrackSceneLayer::components() const
    {
      return components_;
    }

    std::uint32_t TrackSceneLayer::level() const
    {
      return level_;
    }

    void TrackSceneLayer::append_item_geometry(std::size_t item_index, const texture_type* texture,
                                               const vertex_type* vertices, std::uint32_t vertex_count,
                                               const face_type* faces, std::uint32_t face_count)
    {
      auto& item = items_[item_index];
      auto vertex_index = item.vertex_index + item.vertex_count;

      auto component_it = detail::find_component(components_, vertex_index);
    }

    void TrackSceneLayer::append_last_item_geometry(const texture_type* texture,
                                                    const vertex_type* vertices, std::uint32_t vertex_count,
                                                    const face_type* faces, std::uint32_t face_count)
    {
      // If the last component has a matching texture, add the geometry to that component
      if (!components_.empty() && components_.back().texture == texture)
      {
        components_.back().vertex_count += vertex_count;
        components_.back().face_count += face_count;
      }

      // Otherwise, create a new component and add the geometry there.
      else
      {
        Component component;
        component.vertex_index = static_cast<std::uint32_t>(vertices_.size());
        component.vertex_count = vertex_count;
        component.face_index = static_cast<std::uint32_t>(faces_.size());
        component.face_count = face_count;
        component.texture = texture;
        components_.push_back(component);
      }

      auto vertex_index = static_cast<std::uint32_t>(vertices_.size());

      // Simply append the vertices and faces to the back of the buffer...
      vertices_.insert(vertices_.end(), vertices, vertices + vertex_count);

      // Transform the local vertex coordinates to global ones, by adding the vertex index
      // to all of the face's indices.
      auto face_index = faces_.size();
      faces_.insert(faces_.end(), faces, faces + face_count);
      std::transform(faces_.begin() + face_index, faces_.end(), faces_.begin() + face_index,
                     [=](auto face)
      {
        for (auto& index : face.indices)
        {
          index += vertex_index;
        }

        return face;
      });

      // And increment the counters.
      items_.back().vertex_count += vertex_count;
      items_.back().face_count += face_count;
    }
  }
}