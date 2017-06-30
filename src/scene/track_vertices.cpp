/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
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

      for (auto i = 0; i != 4; ++i) vertices[i].color = Colorb(255, 255, 255, 255);

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
        /*
        for (const auto& vertex_array : layer.geometry)
        {
        auto texture_it = textures.find(vertex_array.texture_id);

        if (texture_it != textures.end())
        {
        const auto& texture_def = *texture_it;
        auto mapping_range = texture_lookup(texture_mapping.texture_id(texture_def.id));
        for (const auto& mapping : mapping_range)
        {
        current_texture = mapping.texture;

        auto vertex_transform = [=](auto vertex)
        {
        // Make sure the texture coordinates are updated to match the ones in the texture mapping.
        vertex.texture_coords.x += static_cast<float>(mapping.texture_rect.left - texture_def.image_rect.left);
        vertex.texture_coords.y += static_cast<float>(mapping.texture_rect.top - texture_def.image_rect.top);

        return convert_vertex(vertex, *current_texture);
        };

        auto begin = boost::make_transform_iterator(vertex_array.vertices.begin(), vertex_transform);
        auto end = boost::make_transform_iterator(vertex_array.vertices.end(), vertex_transform);

        vertex_interface.append_vertices(convert_texture(mapping.texture), begin, end, layer.level);
        }
        }
        }
        */

        track_scene.create_layer(&layer);

        std::uint32_t tile_index = 0;
        for (const auto& tile : layer.tiles())
        {
          placed_tiles.clear();
          resources::expand_tiles(&tile, &tile + 1,
                                  tile_library, std::back_inserter(placed_tiles));

          track_scene.add_tile_geometry(&layer, tile_index, placed_tiles.data(), placed_tiles.size());

          ++tile_index;
        }       
      }
    }
  }
}