/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "track_scene.hpp"
#include "track_vertices.hpp"
#include "path_geometry.hpp"

#include "resources/track_layer.hpp"

#include "utility/math_utilities.hpp"

#include <boost/range/iterator_range.hpp>

#include <algorithm>

namespace ts
{
  const std::int32_t region_size = 1024;
  const std::int32_t max_regions = 16;

  namespace scene
  {
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

    TrackSceneLayer::TrackSceneLayer(const resources::TrackLayer* layer, std::uint32_t level_offset)
      : associated_layer_(layer),
        level_offset_(level_offset)        
    {     
      using T = resources::TrackLayerType;
      if (layer->type() == T::BaseTerrain) components_.resize(1);
      else components_.resize(256);

      switch (layer->type())
      {
      case T::BaseTerrain:
        type_ = Type::BaseTerrain;
        break;

      case T::PathStyle:
        type_ = Type::Path;
        break;

      case T::Tiles:
        type_ = Type::Default;
        break;

      default:
        break;
      }
    }

    void TrackSceneLayer::set_type(Type type)
    {
      type_ = type;
    }
    
    TrackSceneLayer::Type TrackSceneLayer::type() const
    {
      return type_;
    }

    const std::vector<resources::Vertex>& TrackSceneLayer::vertices() const
    {
      return vertices_;
    }

    const TrackSceneLayer::ComponentContainer& TrackSceneLayer::component_regions() const
    {
      return components_;
    }

    std::uint32_t TrackSceneLayer::level() const
    {
      return associated_layer_->level() + level_offset_;
    }

    std::uint32_t TrackSceneLayer::z_index() const
    {
      return associated_layer_->z_index();
    }

    const resources::TrackLayer* TrackSceneLayer::associated_layer() const
    {
      return associated_layer_;
    }

    void TrackSceneLayer::clear()
    {
      for (auto& region : components_)
      {
        region.clear();
      }
    }

    void TrackSceneLayer::set_primary_texture(const texture_type* tex)
    {
      primary_texture_ = tex;
    }

    void TrackSceneLayer::set_secondary_texture(const texture_type* tex)
    {
      secondary_texture_ = tex;
    }

    const TrackSceneLayer::texture_type* TrackSceneLayer::primary_texture() const
    {
      return primary_texture_;
    }

    const TrackSceneLayer::texture_type* TrackSceneLayer::secondary_texture() const
    {
      return secondary_texture_;
    }

    Vector2f TrackSceneLayer::primary_texture_tile_size() const
    {
      return primary_texture_tile_size_;
    }

    Vector2f TrackSceneLayer::secondary_texture_tile_size() const
    {
      return secondary_texture_tile_size_;
    }

    void TrackSceneLayer::set_primary_texture_tile_size(Vector2f size)
    {
      primary_texture_tile_size_ = size;
    }

    void TrackSceneLayer::set_secondary_texture_tile_size(Vector2f size)
    {
      secondary_texture_tile_size_ = size;
    }

    namespace detail
    {
      bool region_contains_triangle(FloatRect region, Vector2f t1, Vector2f t2, Vector2f t3)
      {
        auto cross = [=](auto a, auto b, auto p)
        {
          return cross_product(b - a, p - a);
        };

        auto top_left = make_vector2(region.left, region.top);
        auto bottom_right = make_vector2(region.right(), region.bottom());
        auto top_right = make_vector2(bottom_right.x, top_left.y);
        auto bottom_left = make_vector2(top_left.x, bottom_right.y);

        auto test_edge = [=](Vector2f a, Vector2f b)
        {
          return cross(a, b, top_left) < 0 ||
            cross(a, b, bottom_left) < 0 ||
            cross(a, b, bottom_right) < 0 ||
            cross(a, b, top_right) < 0;
        };

        return test_edge(t1, t2) && test_edge(t2, t3) && test_edge(t3, t1);
      }     
    }

    void TrackSceneLayer::store_texture(std::unique_ptr<graphics::Texture> tex)
    {
      stored_texture_ = std::move(tex);
    }

    void TrackSceneLayer::append_geometry(const texture_type* texture,
                                          const vertex_type* vertices, std::uint32_t vertex_count,
                                          const face_type* faces, std::uint32_t face_count)
    {
      if (face_count == 0) return;

      auto vertices_end = vertices + vertex_count;
      auto faces_end = faces + face_count;

      auto vertex_offset = vertices_.size();
      vertices_.insert(vertices_.end(), vertices, vertices_end);

      const auto inv_region_size = 1.0f / region_size;
      for (auto face = faces; face != faces_end; ++face)
      {
        face_type f = *face;
        f.indices[0] += vertex_offset;
        f.indices[1] += vertex_offset;
        f.indices[2] += vertex_offset;

        if (associated_layer_->type() != resources::TrackLayerType::BaseTerrain)
        {
          auto a = vertices[face->indices[0]].position;
          auto b = vertices[face->indices[1]].position;
          auto c = vertices[face->indices[2]].position;

          if (cross_product(b - a, c - a) > 0.0)
          {
            std::swap(a, c);
          }

          FloatRect bounding_rect(a, Vector2f(0, 0));
          bounding_rect = combine(bounding_rect, b);
          bounding_rect = combine(bounding_rect, c);

          auto min_cell_x = static_cast<std::int32_t>(std::floor(bounding_rect.left * inv_region_size));
          auto min_cell_y = static_cast<std::int32_t>(std::floor(bounding_rect.top * inv_region_size));
          auto max_cell_x = static_cast<std::int32_t>(std::floor(bounding_rect.right() * inv_region_size));
          auto max_cell_y = static_cast<std::int32_t>(std::floor(bounding_rect.bottom() * inv_region_size));

          min_cell_x = clamp(min_cell_x, 0, 15);
          max_cell_x = clamp(max_cell_x, 0, 15);
          min_cell_y = clamp(min_cell_y, 0, 15);
          max_cell_y = clamp(max_cell_y, 0, 15);

          for (auto cell_y = min_cell_y; cell_y <= max_cell_y; ++cell_y)
          {
            auto bounding_box = IntRect(min_cell_x * region_size, cell_y * region_size, region_size, region_size);

            for (auto cell_x = min_cell_x; cell_x <= max_cell_x; ++cell_x, bounding_box.left += region_size)
            {
              if (detail::region_contains_triangle(rect_cast<float>(bounding_box), a, b, c))
              {
                auto& entry = components_[cell_y * 16 + cell_x];
                if (entry.empty() || entry.back().texture != texture)
                {
                  Component component;
                  component.bounding_box = IntRect(cell_x * region_size, cell_y * region_size, region_size, region_size);
                  component.texture = texture;

                  entry.push_back(component);
                }

                entry.back().faces.push_back(f);
              }
            }
          }
        }

        else
        {
          auto& entry = components_[0];
          if (entry.empty() || entry.back().texture != texture)
          {
            Component component;
            component.bounding_box = IntRect(0, 0, region_size * max_regions, region_size * max_regions);
            component.texture = texture;
            entry.push_back(component);
          }
          
          entry.back().faces.push_back(f);
        }
      }
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

          auto tex_right = (track_width * 2.0f) / tex_info.texture->size().x;
          auto tex_bottom = (track_height * 2.0f) / tex_info.texture->size().y;

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
            v.z = 0.0f;
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
      auto path_style = path_layer->path_style();

      scene_layer.clear();
      scene_layer.set_primary_texture(nullptr);
      scene_layer.set_secondary_texture(nullptr);     
      if (path_style)
      {        
        const auto& style = path_style->style;
        auto border = texture_mapping_.find(texture_mapping_.texture_id(style.border_texture));
        
        if (style.border_only)
        {
          if (!border.empty())
          {
            scene_layer.set_primary_texture(border.front().texture);
            scene_layer.set_primary_texture_tile_size(style.border_texture_tile_size);
          }
        }

        else
        {
          auto base = texture_mapping_.find(texture_mapping_.texture_id(style.base_texture));
          if (!base.empty())
          {
            scene_layer.set_primary_texture(base.front().texture);
            scene_layer.set_primary_texture_tile_size(style.base_texture_tile_size);
          }

          if (!border.empty())
          {
            scene_layer.set_secondary_texture(border.front().texture);
            scene_layer.set_secondary_texture_tile_size(style.border_texture_tile_size);
          }
        }        

        vertex_cache_.clear();
        face_cache_.clear();

        sf::Image path_texture;
        create_path_geometry(*path_style->path, path_style->style, 0.1f, path_texture, vertex_cache_, face_cache_);        

        if (!face_cache_.empty())
        {
          const texture_type* texture = nullptr;
          if (style.texture_mode != style.Directional)
          {
            auto tex = std::make_unique<graphics::Texture>(graphics::create_texture(path_texture));
            glBindTexture(tex->target(), tex->get());
            glTexParameteri(tex->target(), GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glTexParameteri(tex->target(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(tex->target(), 0);

            texture = tex.get();
            scene_layer.store_texture(std::move(tex));
            scene_layer.set_type(scene_layer.Path);
          }

          else
          {
            texture = scene_layer.primary_texture();
            scene_layer.set_primary_texture(nullptr);
            scene_layer.set_type(scene_layer.Default);
          }

          scene_layer.append_geometry(texture, vertex_cache_.data(), vertex_cache_.size(),
                                      face_cache_.data(), face_cache_.size());
          
        }
      }
    }
  }
}