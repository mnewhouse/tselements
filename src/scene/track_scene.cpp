/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "track_scene.hpp"
#include "track_vertices.hpp"

#include "resources/track_layer.hpp"

#include <boost/range/iterator_range.hpp>

#include <algorithm>

namespace ts
{
  namespace scene
  {
    namespace detail
    {
      template <typename Components>
      auto component_range(Components& components, std::uint32_t face_index, std::uint32_t face_count)
      {
        using std::begin;
        using std::end;

        auto first = std::lower_bound(begin(components), end(components), face_index, 
                                      [](const auto& component, auto face_index)
        {
          return component.face_index + component.face_count < face_index;
        });

        auto last = std::upper_bound(begin(components), end(components), face_index + face_count,
                                     [](auto face_index, const auto& component)
        {
          return face_index < component.face_index + component.face_count;
        });

        // Find the matching component
        return boost::make_iterator_range(first, last);
      }

      template <typename Components>
      auto find_component(Components& component_range, const graphics::Texture* texture)
      {
        using std::begin;
        using std::end;

        return std::find_if(begin(component_range), end(component_range), 
                            [=](const auto& component)
        {
          return component.texture == texture;
        });
      }

      template <typename Items>
      auto find_item(Items& items, std::uint32_t item_index, std::uint32_t level)
      {
        using std::begin;
        using std::end;

        return std::lower_bound(begin(items), end(items), std::make_tuple(level, item_index),
                                [=](const auto& item, auto pair)
        {
          return std::tie(item.level, item.item_index) < pair;
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

    void TrackScene::create_layer(LayerHandle track_layer)
    {
      auto result = layer_map_.insert(std::make_pair(track_layer, TrackSceneLayer(track_layer)));
      if (result.second)
      {
        this->layer_list_.push_back(&result.first->second);
      }
    }

    TrackSceneLayer* TrackScene::find_layer_internal(LayerHandle track_layer)
    {
      auto it = layer_map_.find(track_layer);
      if (it != layer_map_.end()) return &it->second;
      
      return nullptr;
    }

    const TrackSceneLayer* TrackScene::find_layer(LayerHandle track_layer) const
    {
      auto it = layer_map_.find(track_layer);
      if (it != layer_map_.end()) return &it->second;

      return nullptr;
    }

    TrackScene::layer_range TrackScene::layers() const
    {
      return layer_range(layer_list_.data(), layer_list_.data() + layer_list_.size());
    }

    const std::vector<TrackScene::Component>& TrackScene::components() const
    {
      return components_;
    }


    TrackSceneLayer::TrackSceneLayer(const resources::TrackLayer* layer)
      : associated_layer_(layer),
        base_level_(layer->level())
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

    std::uint32_t TrackSceneLayer::base_level() const
    {
      return base_level_;
    }

    const resources::TrackLayer* TrackSceneLayer::associated_layer() const
    {
      return associated_layer_;
    }

    TrackSceneLayer::GeometryUpdate
      TrackSceneLayer::append_item_geometry(std::uint32_t item_index, const texture_type* texture,
                                            const vertex_type* vertices, std::uint32_t vertex_count,
                                            const face_type* faces, std::uint32_t face_count,
                                            std::uint32_t level)
    {
      // Reserve enough capacity so that we can be sure no exceptions will happen later on,
      // guaranteeing strong exception safety.
      faces_.reserve(faces_.size() + face_count);
      vertices_.reserve(vertices_.size() + vertex_count);
      if (components_.capacity() < components_.size() + 8)
      {
        components_.reserve(components_.size() + components_.size() / 2 + 10);
      }

      if (items_.capacity() < items_.size() + 8)
      {
        items_.reserve(items_.size() + items_.size() / 2 + 10);
      }

      auto vertex_index = static_cast<std::uint32_t>(vertices_.size());
      auto face_index = static_cast<std::uint32_t>(faces_.size());

      auto item_it = detail::find_item(items_, item_index, level);

      if (item_it != items_.end())
      {
        // When not inserting to the end of the items list, 
        // use the face and vertex indices of whatever item comes next.
        vertex_index = item_it->vertex_index;
        face_index = item_it->face_index;

        if (item_it->item_index == item_index && item_it->level == level)
        {
          vertex_index += item_it->vertex_count;
          face_index += item_it->face_count;
        }
      }
      
      if (item_it == items_.end() || item_it->item_index != item_index || item_it->level != level)
      {
        // If there's no matching item yet, create a new one.
        ItemInfo entry;
        entry.item_index = item_index;
        entry.level = level;
        entry.vertex_index = vertex_index;
        entry.vertex_count = 0;
        entry.face_index = face_index;
        entry.face_count = 0;
        item_it = items_.insert(item_it, entry);
      }      

      // Increment the geometry counters

      auto component_range = detail::component_range(components_, item_it->face_index, item_it->face_count);

      // Look up the component by the face index.
      auto component_it = detail::find_component(component_range, texture);

      GeometryUpdate geometry_update;
      geometry_update.face_begin = face_index;
      geometry_update.vertex_begin = vertex_index;     

      // If we have a matching component, increment the face count and be done with it.
      if (component_it != component_range.end())
      {
        component_it->face_count += face_count;
      }

      // Otherwise, create a new component at the end of the component range.
      else
      {
        Component new_component;
        new_component.face_index = item_it->face_index + item_it->face_count;
        new_component.face_count = face_count;
        new_component.level = level;
        new_component.texture = texture;        

        component_it = components_.insert(component_range.end(), new_component);
      }

      item_it->vertex_count += vertex_count;
      item_it->face_count += face_count;

      // Transform all components that come after the inserted/modified one.
      std::transform(std::next(component_it), components_.end(), std::next(component_it),
                     [=](Component component)
      {
        component.face_index += face_count;
        return component;
      });
        
      // Now, insert the faces and vertices in the appropriate positions
      auto face_it = faces_.insert(faces_.begin() + face_index, faces, faces + face_count);
      auto vertex_it = vertices_.insert(vertices_.begin() + vertex_index, vertices, vertices + vertex_count);

      // Transform all items that come after the one we modified.
      std::transform(std::next(item_it), items_.end(), std::next(item_it), [=](ItemInfo item)
      {
        item.face_index += face_count;
        item.vertex_index += vertex_count;
        return item;
      });

      // Transform the faces we inserted to match the vertex position
      std::transform(face_it, face_it + face_count, face_it,
                     [=](resources::Face face)
      {
        for (auto& idx : face.indices) idx += vertex_index;
        return face;
      });

      face_it += face_count;

      // And also transform the faces, so that the vertex indices match up again.

      std::transform(face_it, faces_.end(), face_it, 
                     [=](resources::Face face)
      {
        for (auto& idx : face.indices) idx += vertex_count;
        return face;
      });

      geometry_update.face_end = static_cast<std::uint32_t>(faces_.size());
      geometry_update.vertex_end = static_cast<std::uint32_t>(vertices_.size());
      return geometry_update;
    }

    TrackSceneLayer::GeometryUpdate TrackSceneLayer::remove_item_geometry(std::uint32_t index, bool shift)
    {
      auto pred = [=](const ItemInfo& item)
      {
        return item.item_index == index;
      };

      auto item_it = std::find_if(items_.begin(), items_.end(), pred);
      GeometryUpdate geometry_update{};
      if (item_it != items_.end())
      {
        geometry_update.face_begin = item_it->face_index;
        geometry_update.vertex_begin = item_it->vertex_index;
      }

      while (item_it != items_.end())
      {
        auto face_it = faces_.begin() + item_it->face_index;
        auto vertex_it = vertices_.begin() + item_it->vertex_index;

        face_it = faces_.erase(face_it, face_it + item_it->face_count);
        vertex_it = vertices_.erase(vertex_it, vertex_it + item_it->vertex_count);
        
        // Change faces' indices to account for the removal of vertices
        std::transform(face_it, faces_.end(), face_it,
                       [=](resources::Face face)
        {
          for (auto& idx : face.indices) idx -= item_it->vertex_count;
          return face;
        });

        auto component_range = detail::component_range(components_, item_it->face_index, item_it->face_count);
        auto removal_range = component_range;

        // Find the components within the range of the item's faces.
        if (!component_range.empty())
        {
          auto& front = component_range.front();
          front.face_count = item_it->face_index - front.face_index;
          if (front.face_count != 0) removal_range.advance_begin(1);

          if (!component_range.size() != 1)
          {
            auto& back = component_range.back();
            auto index = item_it->face_index + item_it->face_count;
            back.face_count -= index - back.face_index;
            back.face_index = index;

            if (back.face_count != 0) removal_range.drop_back();
          }
        }

        auto component_it = components_.erase(removal_range.begin(), removal_range.end());        

        std::transform(component_it, components_.end(), component_it,
                       [=](Component component)
        {
          component.face_index -= item_it->face_count;
          return component;
        });

        std::transform(std::next(item_it), items_.end(), std::next(item_it),
                       [=](ItemInfo item)
        {
          item.face_index -= item_it->face_count;
          item.vertex_index -= item_it->vertex_count;
          if (shift) --item.item_index;
          return item;
        });

        // Erase vertices and faces belonging to item

        item_it = std::find_if(std::next(item_it), items_.end(), pred);
      }

      items_.erase(std::remove_if(items_.begin(), items_.end(), pred), items_.end());
      
      geometry_update.face_end = static_cast<std::uint32_t>(faces_.size());
      geometry_update.vertex_end = static_cast<std::uint32_t>(vertices_.size());

      return geometry_update;
    }

    namespace detail
    {
      void apply_geometry_update(TrackSceneLayer::GeometryUpdate& result,
                                 const TrackSceneLayer::GeometryUpdate& update)
      {
        if (result.face_begin == result.face_end && result.vertex_begin == result.vertex_end)
        {
          result = update;
        }

        else
        {
          if (update.face_begin < result.face_begin) result.face_begin = update.face_begin;
          if (update.face_end > result.face_end) result.face_end = update.face_end;
          if (update.vertex_begin < result.vertex_begin) result.vertex_begin = update.vertex_begin;
          if (update.vertex_end > result.vertex_end) result.vertex_end = update.vertex_end;
        }
      };
    }

    TrackScene::GeometryUpdate TrackScene::remove_item_geometry(const resources::TrackLayer* layer,
                                                                std::uint32_t item_index)
    {
      if (auto scene_layer = find_layer_internal(layer))
      {
        return scene_layer->remove_item_geometry(item_index);
      }

      return{};
    }

    TrackScene::GeometryUpdate 
      TrackScene::add_tile_geometry(const resources::TrackLayer* layer, std::uint32_t tile_index,
                                    const resources::PlacedTile* expanded_tiles, std::size_t tile_count)
    {
      GeometryUpdate result{};

      if (auto scene_layer = find_layer_internal(layer))
      {
        const auto& tex_mapping = texture_mapping();

        auto expansion_range = boost::make_iterator_range(expanded_tiles, expanded_tiles + tile_count);

        vertex_cache_.clear();
        face_cache_.clear();

        const graphics::Texture* current_texture = nullptr;
        std::uint32_t current_level = 0;

        auto commit_geometry = [&]()
        {
          auto update = scene_layer->append_item_geometry(tile_index, current_texture,
                                                          vertex_cache_.data(), static_cast<std::uint32_t>(vertex_cache_.size()),
                                                          face_cache_.data(), static_cast<std::uint32_t>(face_cache_.size()),
                                                          current_level);

          detail::apply_geometry_update(result, update);
        };

        for (const auto& tile : expansion_range)
        {
          for (const auto& mapping : tex_mapping.find(tex_mapping.tile_id(tile.id)))
          {
            if (current_texture && (mapping.texture != current_texture || tile.level != current_level))
            {
              commit_geometry();

              vertex_cache_.clear();
              face_cache_.clear();
            }

            auto vertices = generate_tile_vertices(tile, *tile.definition,
                                                   mapping.texture_rect, mapping.fragment_offset,
                                                   1.0f / mapping.texture->size());

            auto faces = generate_tile_faces(static_cast<std::uint32_t>(vertex_cache_.size()));

            vertex_cache_.insert(vertex_cache_.end(), vertices.begin(), vertices.end());
            face_cache_.insert(face_cache_.end(), faces.begin(), faces.end());    

            current_texture = mapping.texture;
            current_level = tile.level;
          }
        }

        if (current_texture)
        {
          commit_geometry();
        }
      }

      return result;
    }

    const std::vector<TrackScene::Component>& TrackScene::reload_components()
    {
      components_.clear();
      for (auto scene_layer : layer_list_)
      {
        for (const auto& scene_component : scene_layer->components())
        {
          Component component;
          component.scene_layer = scene_layer;
          component.face_index = scene_component.face_index;
          component.face_count = scene_component.face_count;          
          component.texture = scene_component.texture;
          component.level = scene_component.level;
          component.track_layer = scene_layer->associated_layer();
          components_.push_back(component);
        }
      }

      return sort_components();
    }

    const std::vector<TrackScene::Component>& TrackScene::sort_components()
    {
      std::stable_sort(components_.begin(), components_.end(), 
                       [](const Component& a, const Component& b)
      {
        auto a_level = a.level + a.track_layer->level();
        auto b_level = b.level + b.track_layer->level();
        auto a_index = a.track_layer->z_index();
        auto b_index = b.track_layer->z_index();

        return std::tie(a_level, a_index) < std::tie(b_level, b_index);
      });

      return components_;
    }
  }
}