/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "pattern_builder.hpp"
#include "pattern_loader.hpp"
#include "pattern.hpp"
#include "track.hpp"
#include "track_layer.hpp"
#include "tiles.hpp"
#include "tile_expansion.hpp"

#include "utility/rect.hpp"
#include "utility/vector2.hpp"
#include "utility/rotation.hpp"
#include "utility/transform.hpp"

#include <algorithm>
#include <cmath>

namespace ts
{
  namespace resources
  {
    void apply_pattern(Pattern& dest, const Pattern& source, IntRect rect, 
                       Vector2i position, std::int32_t rotation_degrees);

    Pattern build_track_pattern(const Track& track)
    {
      PatternLoader pattern_loader;
      return build_track_pattern(track, pattern_loader);
    }

    Pattern build_track_pattern(const Track& track, PatternLoader& pattern_loader)
    {
      Pattern pattern(track.size());      

      // Expand all tile groups into plain tiles...
      std::vector<PlacedTile> tile_expansion;

      const auto& layers = track.layers();
      for (const auto& layer : layers)
      {
        expand_tiles(layer.tiles.begin(), layer.tiles.end(), track.tile_library(),
                     std::back_inserter(tile_expansion));
      }

      std::stable_sort(tile_expansion.begin(), tile_expansion.end(),
                       [](const PlacedTile& a, const PlacedTile& b)
      {
        return a.level < b.level;
      });

      // Then for each one, apply it to the resulting pattern.
      for (const auto& placed_tile : tile_expansion)
      {
        const auto* tile_def = placed_tile.definition;
        const auto& tile = placed_tile;

        const auto& source = pattern_loader.load_from_file(tile_def->pattern_file);
        apply_pattern(pattern, source, tile_def->pattern_rect, tile.position, tile.rotation);
      }

      return pattern;
    }

    PatternLoader preload_pattern_files(const resources::Track& track)
    {
      // Look through the track's used tiles and simply load them all.
      PatternLoader pattern_loader;
      const auto& tile_library = track.tile_library();
      const auto& tiles = tile_library.tiles();
      const auto& tile_groups = tile_library.tile_groups();

      for (const auto& layer : track.layers())
      {
        for (const auto& tile : layer.tiles)
        {
          auto tile_def = tiles.find(tile.id);
          auto group_def = tile_groups.find(tile.id);

          if (tile_def != tiles.end())
          {
            pattern_loader.load_from_file(tile_def->pattern_file);
          }
    
          else if (group_def != tile_groups.end())
          {
            for (auto& group : group_def->sub_tiles)
            {
              tile_def = tiles.find(group.id);
              if (tile_def != tiles.end())
              {
                pattern_loader.load_from_file(tile_def->pattern_file);
              }
            }
          }
        }
      }

      return pattern_loader;
    }

    void apply_pattern(Pattern& dest, const Pattern& source,
                       IntRect rect, Vector2i position, std::int32_t rotation_degrees)
    {
      auto rotation = degrees(static_cast<double>(rotation_degrees));
      double sin = -std::sin(rotation.radians());
      double cos = std::cos(rotation.radians());

      Vector2i world_size(dest.size().x, dest.size().y);
      Vector2i pattern_size(source.size().x, dest.size().y);
      Vector2i source_size(rect.width, rect.height);

      Vector2i dest_size;
      {
        double x = source_size.x * 0.5;
        double y = source_size.y * 0.5;

        double cx = x * cos;
        double cy = y * cos;
        double sx = x * sin;
        double sy = y * sin;

        double half_width = std::abs(cx) + std::abs(sy);
        double half_height = std::abs(cy) + std::abs(sx);

        dest_size.x = static_cast<std::int32_t>(std::ceil(half_width * 2.0));
        dest_size.y = static_cast<std::int32_t>(std::ceil(half_height * 2.0));
      }

      auto source_center = vector2_cast<double>(source_size) * 0.5;

      if (rect.right() > pattern_size.x) rect.width = pattern_size.x - rect.left;
      if (rect.bottom() > pattern_size.y) rect.height = pattern_size.y - rect.top;

      auto int_pos = vector2_round<std::int32_t>(position);
      auto offset = int_pos - source_size / 2;

      auto start = (source_size - dest_size) / 2 - 1;
      auto end = start + dest_size + 2;

      for (std::int32_t y = start.y; y <= end.y; ++y)
      {
        Vector2<double> dest_point(0.0, y - source_center.y);

        for (std::int32_t x = start.x; x <= end.x; ++x)
        {
          dest_point.x = x - source_center.x;

          std::int32_t absolute_x = x + offset.x;
          std::int32_t absolute_y = y + offset.y;

          if (absolute_x >= 0 && absolute_y >= 0 && absolute_x < world_size.x && absolute_y < world_size.y)
          {
            auto source_point = transform_point<double>(dest_point, sin, cos) + source_center;
            source_point += { 0.5, 0.5 };

            auto point = vector2_cast<std::int32_t>(source_point);
            if (source_point.x >= 0.0 && source_point.y >= 0.0 && 
                point.x < rect.width && point.y < rect.height)
            {
              // For source terrain id = 0, don't overwrite the destination pattern.
              if (auto terrain = source(point.x + rect.left, point.y + rect.top))
              {
                dest(absolute_x, absolute_y) = terrain;
              }
            }
          }
        }
      }
    }
  }
}