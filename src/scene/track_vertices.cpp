/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "track_vertices.hpp"
#include "texture_mapping.hpp"
#include "track_scene.hpp"

#include "resources/track.hpp"
#include "resources/tile_library.hpp"
#include "resources/texture_library.hpp"
#include "resources/tiles.hpp"
#include "resources/tile_expansion.hpp"

namespace ts
{
  namespace scene
  {
    std::array<resources::Vertex, 4>
      generate_tile_vertices(const resources::Tile& tile, const resources::TileDefinition& tile_def,
                             const IntRect& texture_rect, Vector2i fragment_offset, Vector2f texture_scale)
    {
      std::array<resources::Vertex, 4> vertices;

      const auto& image_rect = tile_def.image_rect;
      const auto& pattern_rect = tile_def.pattern_rect;

      auto tile_rotation = degrees(static_cast<float>(tile.rotation));

      auto transform = make_transformation(tile_rotation);

      auto pattern_size = vector2_cast<float>(size(pattern_rect));

      auto scale = make_vector2(pattern_size.x / image_rect.width, pattern_size.y / image_rect.height);
      auto center = make_vector2(pattern_size.x * 0.5f, pattern_size.y * 0.5f);

      auto tile_position = vector2_cast<float>(tile.position);
      auto scaled_offset = (fragment_offset + make_vector2(0.5f, 0.5f)) * scale;

      auto fragment_size = vector2_cast<float>(size(texture_rect)) - make_vector2(1.0f, 1.0f);

      auto top_left = scaled_offset - center;
      auto bottom_right = top_left + fragment_size * scale;
      auto top_right = make_vector2(bottom_right.x, top_left.y);
      auto bottom_left = make_vector2(top_left.x, bottom_right.y);

      auto tex_coords = rect_cast<float>(texture_rect);

      vertices[0].position = tile_position + transform_point(top_left, transform);
      vertices[1].position = tile_position + transform_point(bottom_left, transform);
      vertices[2].position = tile_position + transform_point(top_right, transform);
      vertices[3].position = tile_position + transform_point(bottom_right, transform);

      auto tex_left = tex_coords.left + 0.5f;
      auto tex_top = tex_coords.top + 0.5f;
      auto tex_right = tex_coords.right() - 0.5f;
      auto tex_bottom = tex_coords.bottom() - 0.5f;

      vertices[0].texture_coords = make_vector2(tex_left, tex_top) * texture_scale;
      vertices[1].texture_coords = make_vector2(tex_left, tex_bottom) * texture_scale;
      vertices[2].texture_coords = make_vector2(tex_right, tex_top) * texture_scale;
      vertices[3].texture_coords = make_vector2(tex_right, tex_bottom) * texture_scale;

      for (auto i = 0; i != 4; ++i)
      {
        vertices[i].color = Colorb(255, 255, 255, 255);
        vertices[i].z = 0.0f;
      }

      return vertices;
    }

    std::array<resources::Face, 2> generate_tile_faces(std::uint32_t base_index)
    {
      return
      { {
        { base_index, base_index + 1, base_index + 2 },
        { base_index + 1, base_index + 2, base_index + 3 }
      } };
    }


    void build_track_vertices(const resources::Track& track, TrackScene& track_scene)

    {
      const auto& tile_library = track.tile_library();
      const auto& texture_library = track.texture_library();
      const auto& texture_mapping = track_scene.texture_mapping();

      const auto& textures = texture_library.textures();

      boost::optional<TextureMapping::texture_type> current_texture = boost::none;
      Vector2f texture_scale;

      std::vector<resources::PlacedTile> placed_tiles;
      std::vector<resources::Vertex> vertex_cache;

      auto texture_lookup = [&](std::size_t resource_id)
      {
        if (current_texture) return texture_mapping.find(resource_id, *current_texture);

        return texture_mapping.find(resource_id);
      };

      for (const auto& layer : track.layers())
      {
        if (auto tiles = layer.tiles())
        {
          for (const auto& tile : *tiles)
          {
            placed_tiles.clear();
            resources::expand_tiles(&tile, &tile + 1,
                                    tile_library, std::back_inserter(placed_tiles));

            track_scene.add_tile_geometry(&layer, placed_tiles.data(), placed_tiles.size());
          }
        }

        else if (auto path_style = layer.path_style())
        {
          track_scene.rebuild_path_layer_geometry(&layer);
        }

        else if (auto base_terrain = layer.base_terrain())
        {
          track_scene.rebuild_base_terrain_geometry(&layer);
        }
      }
    }
  }
}