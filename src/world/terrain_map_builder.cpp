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

      const auto& tile_library = track.tile_library();
      const auto& texture_library = track.texture_library().textures();

      std::uint32_t z_index = 0;
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

            TerrainMapComponent component;
            component.data = face;
            component.level = layer.level();
            component.z_index = z_index;

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
            pattern.alpha = 255;
            pattern.pattern = &pattern_store.load_from_file(tile_def->pattern_file);            
            pattern.pattern_rect = tile_def->pattern_rect;
            pattern.position = tile.position;
            pattern.transformation = map_components::pattern_transformation_lookup[normalized_rotation];           

            TerrainMapComponent component;
            component.level = tile.level + layer.level();
            component.z_index = z_index++;
            component.data = pattern;

            components.push_back(component);
          }
        }

        else if (auto* path_style = layer.path_style())
        {
          /*
          for (const auto& style : path_styles->styles)
          {
            outline_points.clear();
            auto outline_indices = scene::generate_path_outline(*path_styles->path, style, 0.5f, outline_points);           

            for (const auto& border : style.border_styles)
            {
              auto texture_it = texture_library.find(border.texture_id);
              if (texture_it == texture_library.end()) continue;

              path_vertices.clear();
              path_faces.clear();

              scene::create_border_geometry(outline_points, outline_indices, border, {}, path_vertices, path_faces);

              add_path_component(border.terrain_id);
              ++z_index;
            }

            path_vertices.clear();
            path_faces.clear();
            scene::create_base_geometry(outline_points, outline_indices, style, {}, path_vertices, path_faces);

            auto texture_it = texture_library.find(style.texture_id);
            if (texture_it != texture_library.end())
            {
              add_path_component(style.terrain_id);
              ++z_index;
            }
          }
          */
        }

        else if (auto* base_terrain = layer.base_terrain())
        {
          auto texture_it = texture_library.find(base_terrain->texture_id);
          if (texture_it != texture_library.end())
          {
            auto track_size = track.size();

            map_components::Base base;
            base.alpha = 255;
            base.rect = { 0, 0, track_size.x, track_size.y };
            base.terrain_id = base_terrain->terrain_id;

            TerrainMapComponent component;
            component.data = base;
            component.level = layer.level();
            component.z_index = z_index;

            components.push_back(component);
            ++z_index;
          }
        }
        


        /*
        else if (layer.type() == resources::TrackLayerType::Geometry)
        {
          for (const auto& geometry : layer.geometry())
          {
            for (const auto& face : geometry.faces)
            {              
              map_components::Face face_data;
              face_data.alpha = 255; // TODO

              auto a = geometry.vertices[face.indices[0]];
              auto b = geometry.vertices[face.indices[1]];
              auto c = geometry.vertices[face.indices[2]];

              face_data.vertices =
              { {
                vector2_cast<std::int32_t>(a.position),
                vector2_cast<std::int32_t>(b.position),
                vector2_cast<std::int32_t>(c.position)
              } };

              
              TerrainMapComponent component;
              component.level = layer.level() + geometry.level;
              component.z_index = z_index++;              
              component.data = face_data;

              components.push_back(component);
            }
          }
        }
        */
      }

      return TerrainMap(components, std::move(pattern_store));      
    }
  }
}