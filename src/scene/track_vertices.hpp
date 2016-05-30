/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_VERTICES_HPP_9901203
#define TRACK_VERTICES_HPP_9901203

#include "texture_mapping.hpp"

#include "utility/vertex_interface.hpp"
#include "utility/color.hpp"
#include "utility/vector2.hpp"

#include "resources/track.hpp"
#include "resources/track_layer.hpp"
#include "resources/tile_library.hpp"
#include "resources/texture_library.hpp"
#include "resources/tile_expansion.hpp"

#include <boost/optional.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <algorithm>
#include <cmath>

namespace ts
{
  namespace scene
  {
    // Takes a tile and its definition, generate exactly four vertices of type resource::Vertex
    // and write these to the given output iterator.
    template <typename OutIt>
    void generate_tile_vertices(const resources::Tile& tile, const resources::TileDefinition& tile_def,
                                const IntRect& texture_rect, Vector2i fragment_offset, OutIt out)
    {
      resources::Vertex vertices[6];

      const auto& image_rect = tile_def.image_rect;
      const auto& pattern_rect = tile_def.pattern_rect;

      auto tile_rotation = degrees(static_cast<float>(tile.rotation));

      float sin = std::sin(tile_rotation.radians());
      float cos = std::cos(tile_rotation.radians());

      auto pattern_width = static_cast<float>(pattern_rect.width);
      auto pattern_height = static_cast<float>(pattern_rect.height);

      auto scale = make_vector2(pattern_width / image_rect.width, pattern_height / image_rect.height);
      auto center = make_vector2(pattern_width * 0.5f, pattern_height * 0.5f);

      auto tile_position = vector2_cast<float>(tile.position);
      auto scaled_offset = vector2_cast<float>(fragment_offset) * scale;

      auto fragment_size = vector2_cast<float>(make_vector2(texture_rect.width, texture_rect.height));

      auto top_left = scaled_offset - center;
      auto bottom_right = top_left + fragment_size * scale;
      auto top_right = make_vector2(bottom_right.x, top_left.y);
      auto bottom_left = make_vector2(top_left.x, bottom_right.y);

      auto tex_coords = rect_cast<float>(texture_rect);

      vertices[0].position = tile_position + transform_point(top_left, sin, cos);
      vertices[1].position = tile_position + transform_point(bottom_left, sin, cos);
      vertices[2].position = tile_position + transform_point(bottom_right, sin, cos);
      vertices[3].position = tile_position + transform_point(top_right, sin, cos);
      vertices[4].position = vertices[0].position;
      vertices[5].position = vertices[2].position;

      auto tex_left = tex_coords.left + 0.5f;
      auto tex_top = tex_coords.top + 0.5f;
      auto tex_right = tex_coords.right() - 0.5f;
      auto tex_bottom = tex_coords.bottom() - 0.5f;

      vertices[0].texture_coords = make_vector2(tex_left, tex_top);
      vertices[1].texture_coords = make_vector2(tex_left, tex_bottom);
      vertices[2].texture_coords = make_vector2(tex_right, tex_bottom);

      vertices[3].texture_coords = make_vector2(tex_right, tex_top);
      vertices[4].texture_coords = vertices[0].texture_coords;
      vertices[5].texture_coords = vertices[2].texture_coords;

      std::copy_n(vertices, 6, out);
    }

    // This function takes a track and generates the vertices required to display it.
    // Some notes: 
    // * TextureConv must take a parameter of type 'MappingTexture' and return a 'TextureType'
    // * VertexConv must take a parameter of type resources::Vertex and return a 'VertexType'
    // If no conversions are needed or the conversion is implicit, these parameters may be omitted.
    template <typename TextureType, typename VertexType, typename MappingTexture,
      typename TextureConv, typename VertexConv>
    void build_track_vertices(utility::VertexInterface<TextureType, VertexType>& vertex_interface, 
                              const resources::Track& track, const TextureMapping<MappingTexture>& texture_mapping,
                              TextureConv convert_texture, VertexConv convert_vertex)

    {
      const auto& tile_library = track.tile_library();
      const auto& texture_library = track.texture_library();

      const auto& textures = tile_library.tiles();

      boost::optional<MappingTexture> current_texture = boost::none;

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

        resources::expand_tiles(layer.tiles.begin(), layer.tiles.end(),
                                tile_library, std::back_inserter(placed_tiles));
      }

      using resources::PlacedTile;
      std::stable_sort(placed_tiles.begin(), placed_tiles.end(), 
                       [](const PlacedTile& a, const PlacedTile& b)
      {
        return a.level < b.level;
      });


      // Now, we need to generate vertices for all tiles in the vector.
      // Loop through the tiles in the list, look up the texture, and generate the vertices into
      // our vertex cache. While the texture handle remains the same, keep adding to the vertex cache,
      // and once it changes, flush the vertices into our output object.
      for (const auto& placed_tile : placed_tiles)
      {        
        auto mapping_range = texture_lookup(texture_mapping.tile_id(placed_tile.id));
        for (const auto& mapping : mapping_range)
        {
          current_texture = mapping.texture;

          vertex_cache.clear();
          generate_tile_vertices(placed_tile, *placed_tile.definition, mapping.texture_rect,
                                 mapping.fragment_offset, std::back_inserter(vertex_cache));

          auto conversion_func = std::bind(convert_vertex, std::placeholders::_1, mapping.texture);

          auto begin = boost::make_transform_iterator(vertex_cache.begin(), conversion_func);
          auto end = boost::make_transform_iterator(vertex_cache.end(), conversion_func);

          vertex_interface.append_vertices(convert_texture(mapping.texture), begin, end, placed_tile.level);
        }

      }
    }

    template <typename TextureType, typename VertexType, typename MappingTexture>
    void build_track_vertices(utility::VertexInterface<TextureType, VertexType>& vertex_interface,
                              const resources::Track& track, const TextureMapping<MappingTexture>& texture_mapping)
    {
      auto texture_conv = [](auto t) { return t; };
      auto vertex_conv = [](auto v) { return v; };

      build_track_vertices(vertex_interface, track, texture_mapping, texture_conv, vertex_conv);
    }
  }
}

#endif
