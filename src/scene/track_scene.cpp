/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "track_scene.hpp"
#include "track_vertices.hpp"
#include "path_geometry.hpp"

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

    TrackSceneLayer& TrackScene::scene_layer(LayerHandle track_layer, std::uint32_t level)
    {
      auto key = std::make_pair(track_layer, level);
      auto result = layer_map_.insert(std::make_pair(key, TrackSceneLayer(track_layer, level)));
      if (result.second)
      {
        this->layer_list_.push_back(&result.first->second);
      }

      return result.first->second;
    }

    TrackSceneLayer* TrackScene::find_layer_internal(LayerHandle track_layer, std::uint32_t level)
    {
      auto it = layer_map_.find(std::make_pair(track_layer, level));
      if (it != layer_map_.end()) return &it->second;
      
      return nullptr;
    }

    const TrackSceneLayer* TrackScene::find_layer(LayerHandle track_layer, std::uint32_t level) const
    {      
      auto it = layer_map_.find(std::make_pair(track_layer, level));
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


    TrackSceneLayer::TrackSceneLayer(const resources::TrackLayer* layer, std::uint32_t base_level, std::int32_t patch_size)
      : associated_layer_(layer),
        base_level_(layer->level() + base_level),
        patch_size_(patch_size)
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

    const TrackSceneLayer::ComponentContainer& TrackSceneLayer::components() const
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

    void TrackSceneLayer::clear()
    {
      components_.clear();      
    }

    void TrackSceneLayer::append_geometry(const texture_type* texture,
                                          const vertex_type* vertices, std::uint32_t vertex_count,
                                          const face_type* faces, std::uint32_t face_count)
    {
      if (face_count == 0) return;

      auto vertices_end = vertices + vertex_count;
      auto faces_end = faces + face_count;

      auto face_index = faces_.size();      
      if (components_.empty() || components_.back().texture != texture)
      {
        components_.emplace_back();        
        components_.back().face_index = face_index;
        components_.back().texture = texture;        
      }

      auto vertex_offset = vertices_.size();
      vertices_.insert(vertices_.end(), vertices, vertices_end);

      auto& component = components_.back();
      faces_.insert(faces_.end(), faces, faces_end);

      auto face_it = faces_.begin() + face_index;
      std::transform(face_it, faces_.end(), face_it,
                     [=](face_type f)
      {
        for (auto& i : f.indices) i += vertex_offset;

        return f;
      });   

      component.face_count += face_count;
    }

    void TrackScene::add_base_terrain_geometry(const resources::TrackLayer* layer)
    {
      if (auto base_terrain = layer->base_terrain())
      {        
        auto& scene_layer = this->scene_layer(layer);

        scene_layer.clear();

        const auto& tex_mapping = texture_mapping();
        auto tex_range = tex_mapping.find(tex_mapping.texture_id(base_terrain->texture_id));
        if (!tex_range.empty())
        {
          auto& tex_info = tex_range.front();                 

          auto track_width = static_cast<float>(track_size_.x);
          auto track_height = static_cast<float>(track_size_.y);          

          auto tex_right = (track_width * 4.0f) / tex_info.texture->size().x;
          auto tex_bottom = (track_height * 4.0f) / tex_info.texture->size().y;

          resources::Vertex vertices[4];
          vertices[0].position = { 0.0f, 0.0f };
          vertices[1].position = { track_width, 0.0f };
          vertices[2].position = { track_width, track_height };
          vertices[3].position = { 0.0f, track_height };

          vertices[0].texture_coords = { 0.0f, 0.0f };
          vertices[1].texture_coords = { tex_right,  0.0f };
          vertices[2].texture_coords = { tex_right, tex_bottom };
          vertices[3].texture_coords = { 0.0f, tex_bottom };

          for (auto& v : vertices)
          {
            v.color = base_terrain->color;
          }

          resources::Face faces[2] = 
          {
            { 0, 1, 2 }, { 0, 2, 3 }
          };

          scene_layer.append_geometry(tex_info.texture, vertices, 4, faces, 2);
        }
      }
    }

    void TrackScene::add_tile_geometry(const resources::TrackLayer* layer,
                                       const resources::PlacedTile* expanded_tiles, std::size_t tile_count)
    {
      const auto& tex_mapping = texture_mapping();
      auto expansion_range = boost::make_iterator_range(expanded_tiles, expanded_tiles + tile_count);

      for (const auto& tile : expansion_range)
      {
        for (const auto& mapping : tex_mapping.find(tex_mapping.tile_id(tile.id)))
        {
          auto vertices = generate_tile_vertices(tile, *tile.definition,
                                                 mapping.texture_rect, mapping.fragment_offset,
                                                 1.0f / mapping.texture->size());

          auto faces = generate_tile_faces(0);

          auto& scene_layer = this->scene_layer(layer, tile.level);

          scene_layer.append_geometry(mapping.texture,
                                      vertices.data(), static_cast<std::uint32_t>(vertices.size()),
                                      faces.data(), static_cast<std::uint32_t>(faces.size()));
        }
      }
    }

    void TrackScene::rebuild_tile_layer_geometry(const resources::TrackLayer* tile_layer,
                                                 const resources::PlacedTile* expanded_tile, std::size_t count)
    {
      for (auto scene_layer : layer_list_)
      {
        if (scene_layer->associated_layer() == tile_layer)
        {
          scene_layer->clear();
        }
      }

      add_tile_geometry(tile_layer, expanded_tile, count);
    }

    void TrackScene::rebuild_path_layer_geometry(const resources::TrackLayer* path_layer)
    {
      // Generate vertices and faces and add them to the list of components
      auto& scene_layer = this->scene_layer(path_layer, 0);
      auto path_styles = path_layer->path_styles();

      scene_layer.clear();
      
      if (path_styles)
      {
        for (auto& style : path_styles->styles)
        {
          auto texture_range = texture_mapping_.find(texture_mapping_.texture_id(style.texture_id));

          path_outline_cache_.clear();
          auto outline_indices = generate_path_outline(*path_styles->path, style, 0.15f, path_outline_cache_);

          for (auto& border_style : style.border_styles)
          {
            vertex_cache_.clear();
            face_cache_.clear();

            auto border_texture = texture_mapping_.find(texture_mapping_.texture_id(border_style.texture_id));
            if (!border_texture.empty())
            {
              const auto& texture_info = border_texture.front();
              auto texture_size = vector2_cast<float>(texture_info.texture->size());

              create_border_geometry(path_outline_cache_, outline_indices, border_style, texture_size,
                                     vertex_cache_, face_cache_);

              scene_layer.append_geometry(texture_info.texture,
                                          vertex_cache_.data(), vertex_cache_.size(),
                                          face_cache_.data(), face_cache_.size());
            }
          }

          if (!texture_range.empty())
          {
            auto& texture_info = texture_range.front();
            auto texture_size = vector2_cast<float>(texture_info.texture->size());

            vertex_cache_.clear();
            face_cache_.clear();

            create_base_geometry(path_outline_cache_, outline_indices, style, texture_size, 
                                 vertex_cache_, face_cache_);        
            
            scene_layer.append_geometry(texture_info.texture,
                                        vertex_cache_.data(), vertex_cache_.size(),
                                        face_cache_.data(), face_cache_.size());
          }          
        }
      }
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
        auto a_level = a.scene_layer->base_level();
        auto b_level = b.scene_layer->base_level();
        auto a_index = a.track_layer->z_index();
        auto b_index = b.track_layer->z_index();

        return std::tie(a_level, a_index) < std::tie(b_level, b_index);
      });

      return components_;
    }
  }
}