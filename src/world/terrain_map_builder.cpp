/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "terrain_map_builder.hpp"
#include "terrain_map.hpp"
#include "terrain_map_pattern_transformations.hpp"

#include "resources/track.hpp"
#include "resources/tile_expansion.hpp"

#include "scene/path_geometry.hpp"

#include <vector>
#include <array>

namespace ts
{
  namespace world
  {
    TerrainMap build_terrain_map(const resources::Track& track)
    {
      resources::PatternStore pattern_store;

      std::vector<TerrainMapComponent> components;
      std::vector<resources::PlacedTile> tile_expansion;

      std::vector<scene::PathFace> path_faces;
      std::vector<scene::PathVertex> path_vertices;

      const auto& tile_library = track.tile_library();
      const auto& texture_library = track.texture_library().textures();

      resources::TerrainId base_terrain_id = 0;
      for (const auto& layer : track.layers())
      {
        auto add_path_component = [&](auto terrain_id)
        {
          for (auto& path_face : path_faces)
          {
            map_components::Face face;
            face.alpha = 255;
            face.mask = nullptr;
            face.terrain_id = terrain_id;

            std::transform(path_face.indices.begin(), path_face.indices.end(), face.vertices.begin(),
                           [&](std::uint32_t idx)
            {
              return vector2_cast<std::int32_t>(path_vertices[idx].position);
            });

            if (cross_product(face.vertices[1] - face.vertices[0], face.vertices[2] - face.vertices[0]) > 0.0)
            {
              std::swap(face.vertices[0], face.vertices[2]);
            }

            TerrainMapComponent component;
            component.data = face;
            component.level = layer.level();
            components.push_back(component);
          }
        };

        if (auto* tiles = layer.tiles())
        {          
          tile_expansion.clear();
          resources::expand_tiles(tiles->begin(), tiles->end(), tile_library, std::back_inserter(tile_expansion));

          for (const auto& tile : tile_expansion)
          {
            auto tile_def = tile.definition;

            auto normalized_rotation = tile.rotation;
            if (normalized_rotation >= 360) normalized_rotation %= 360;
            else if (normalized_rotation < 0) normalized_rotation += 360 * ((normalized_rotation - 359) / -360);

            map_components::Pattern pattern;
            pattern.pattern = &pattern_store.load_from_file(tile_def->pattern_file);            
            pattern.rect = tile_def->pattern_rect;
            pattern.position = tile.position;
            pattern.transformation = map_components::pattern_transformation_lookup[normalized_rotation];           

            TerrainMapComponent component;
            component.level = tile.level + layer.level();
            component.data = pattern;

            components.push_back(component);
          }
        }

        else if (auto* path_style = layer.path_style())
        {
          path_vertices.clear();
          path_faces.clear();

          float max_width;
          scene::create_path_geometry(*path_style->path, path_style->style, 0.75f, max_width, path_vertices, path_faces);            
          add_path_component(path_style->style.terrain_id);     
        }

        else if (auto* base_terrain = layer.base_terrain())
        {
          auto texture_it = texture_library.find(base_terrain->texture_id);
          if (texture_it != texture_library.end())
          {
            base_terrain_id = base_terrain->terrain_id;
          }
        }
      }

      return TerrainMap(components, std::move(pattern_store), track.size(), base_terrain_id);      
    }
  }
}