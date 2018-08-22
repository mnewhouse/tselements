/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "terrain_map.hpp"

#include "resources/terrain_library.hpp"

#include "utility/interpolate.hpp"
#include "utility/triangle_utilities.hpp"
#include "utility/math_utilities.hpp"

namespace ts
{
  namespace world
  {
    namespace map_components
    {
      IntRect bounding_box(const Pattern& pattern)
      {
        const auto& rect = pattern.rect;
        auto width = std::abs(rect.width * pattern.transformation[0]) +
          std::abs(rect.height * pattern.transformation[1]);
        
        auto height = std::abs(rect.width * pattern.transformation[1]) +
          std::abs(rect.height * pattern.transformation[0]);

        width >>= 16;
        height >>= 16;

        auto top_left = pattern.position - make_vector2(width >> 1, height >> 1);       
        return IntRect(top_left, make_vector2(width, height));
      }

      IntRect bounding_box(const Face& face)
      {
        auto min = face.vertices[0], max = min;
        for (int i = 1; i < 3; ++i)
        {
          auto p = face.vertices[i];
          if (p.x < min.x) min.x = p.x;
          if (p.x > max.x) max.x = p.x;
          if (p.y < min.y) min.y = p.y;
          if (p.y > max.y) max.y = p.y;
        }

        return IntRect(min.x, min.y, max.x - min.x, max.y - min.y);
      }

      Vector2i calculate_local_coords(const Pattern& pattern, Vector2i position)
      {
        auto coords = (position - pattern.position);
        const auto& t = pattern.transformation;

        auto result = make_vector2(
          (coords.x * t[1] + coords.y * t[0]) >> 16,
          (coords.y * t[1] - coords.x * t[0]) >> 16
        );

        result.x += pattern.rect.width >> 1;
        result.y += pattern.rect.height >> 1;
        return result;
      }


      bool region_contains(IntRect region, const Face& face)
      {
        return region_contains_triangle(region, face.vertices[0], face.vertices[1], face.vertices[2]);
      }

      bool region_contains(IntRect region, const Pattern& pattern)
      {
        auto top_left = calculate_local_coords(pattern, { region.left, region.top });
        auto bottom_left = calculate_local_coords(pattern, { region.left, region.bottom() });
        auto bottom_right = calculate_local_coords(pattern, { region.right(), region.bottom() });
        auto top_right = calculate_local_coords(pattern, { region.right(), region.top });

        auto x_limits = std::minmax({ top_left.x, bottom_left.x, bottom_right.x, top_right.x });
        auto y_limits = std::minmax({ top_left.y, bottom_left.y, bottom_right.y, top_right.y });

        auto rect = IntRect(x_limits.first, y_limits.first,
                            x_limits.second - x_limits.first, y_limits.second - y_limits.first);

        return intersects(IntRect(0, 0, pattern.rect.width, pattern.rect.height), rect);
      }

      TerrainDescriptor terrain_at(const Pattern& pattern, Vector2i position)
      {
        const auto& pat_rect = pattern.rect;
        auto coords = calculate_local_coords(pattern, position);

        if (coords.x >= 0 && coords.y >= 0 && coords.x < pat_rect.width && coords.y < pat_rect.height)
        {
          auto terrain_id = (*pattern.pattern)(coords.x + pat_rect.left, coords.y + pat_rect.top);
          return
          {
            static_cast<resources::TerrainId>(terrain_id),
            static_cast<std::uint32_t>(terrain_id == 0 ? 0 : 255)
          };
        }

        return {};
      }

      TerrainDescriptor terrain_at(const Face& face, Vector2i position)
      {
        auto sign = [](auto a, auto b, auto p)
        {
          return cross_product(b - a, p - a) <= 0;
        };

        const auto& v = face.vertices;
        if (sign(v[0], v[1], position) && sign(v[1], v[2], position) && sign(v[2], v[0], position))
        {
          return{ face.terrain_id, face.alpha };
        }

        return{};
      }

      TerrainDescriptor terrain_at(const Base& base, Vector2i position)
      {
        if (contains(base.rect, position))
        {
          return{ base.terrain_id, base.alpha };
        }

        return{};
      }
    }

    namespace detail
    {
      TerrainDescriptor terrain_at(const TerrainMapComponent& component, Vector2i position)
      {
        return boost::apply_visitor([=](const auto& data)
        {
          return map_components::terrain_at(data, position);
        }, component.data);
      }

      resources::TerrainDefinition interpolate_terrain(const resources::TerrainDefinition& first,
                                                       const resources::TerrainDefinition& second,
                                                       std::uint8_t alpha)
      {
        if (alpha == 255)
        {
          return second;
        }

        const auto real_alpha = alpha / 255.0;

        resources::TerrainDefinition result;
        result.acceleration = interpolate_linearly(first.acceleration, second.acceleration, real_alpha);
        result.braking = interpolate_linearly(first.braking, second.braking, real_alpha);
        result.cornering = interpolate_linearly(first.cornering, second.cornering, real_alpha);
        result.jump = interpolate_linearly(first.jump, second.jump, real_alpha);
        result.rolling_resistance = interpolate_linearly(first.rolling_resistance, second.rolling_resistance, real_alpha);
        result.roughness = interpolate_linearly(first.roughness, second.roughness, real_alpha);
        result.traction = interpolate_linearly(first.traction, second.traction, real_alpha);
        result.sliding_traction = interpolate_linearly(first.sliding_traction, second.sliding_traction, real_alpha);
        result.id = first.id;

        auto interpolate_color = [](std::int32_t a, std::int32_t b, std::int32_t t)
        {
          return a + (((b - a) * (t + 1)) >> 8);
        };

        result.color.r = interpolate_color(first.color.r, second.color.r, alpha);
        result.color.g = interpolate_color(first.color.g, second.color.g, alpha);
        result.color.b = interpolate_color(first.color.b, second.color.b, alpha);
        result.color.a = interpolate_color(first.color.a, second.color.a, alpha);

        return result;
        
      }
    }

    TerrainMap::TerrainMap(std::vector<TerrainMapComponent> components, resources::PatternStore pattern_store, 
                           Vector2i track_size, resources::TerrainId base_terrain)
      : terrain_components_(std::move(components)),
      pattern_store_(std::move(pattern_store)),
      track_size_(track_size),
      base_terrain_(base_terrain)
    {
      while (cell_bits_ > 3 && ((track_size_.x * track_size_.y) >> (cell_bits_ * 2)) < 4096)
      {
        --cell_bits_;
      }
      auto cell_size = (1 << cell_bits_);

      num_cells_.x = (track_size_.x + cell_size - 1) >> cell_bits_;
      num_cells_.y = (track_size_.y + cell_size - 1) >> cell_bits_;

      std::uint32_t max_level = 0;

      struct CellComponent
      {
        std::int32_t cell_x;
        std::int32_t cell_y;
        std::uint32_t component_idx;
        std::uint32_t level;
      };

      std::vector<CellComponent> cell_components;
      for (std::uint32_t idx = 0; idx < terrain_components_.size(); ++idx)
      {
        auto& component = terrain_components_[idx];
        if (component.level > max_level) max_level = component.level;

        IntRect bounding_box = boost::apply_visitor([this](const auto& v)
        {
          return intersection(map_components::bounding_box(v), IntRect(Vector2i(), track_size_));
        }, component.data);

        auto min_cell_x = bounding_box.left >> cell_bits_;
        auto min_cell_y = bounding_box.top >> cell_bits_;
        auto max_cell_x = bounding_box.right() >> cell_bits_;
        auto max_cell_y = bounding_box.bottom() >> cell_bits_;

        for (auto y = min_cell_y; y <= max_cell_y; ++y)
        {
          IntRect region(min_cell_x << cell_bits_, y << cell_bits_, cell_size, cell_size);
          for (auto x = min_cell_x; x <= max_cell_x; ++x, region.left += cell_size)
          {
            bool contained = boost::apply_visitor([=](const auto& v)
            {
              return region_contains(region, v);
            }, component.data);

            if (contained)
            {
              cell_components.push_back({ x, y, idx, component.level });
            }
          }
        }
      }

      auto num_levels = max_level + 1;
      terrain_cells_.resize(num_levels * num_cells_.x * num_cells_.y);

      std::sort(cell_components.begin(), cell_components.end(),
                [](const CellComponent& a, const CellComponent& b)
      {
        return std::tie(a.level, a.cell_y, a.cell_x, b.component_idx) <
          std::tie(b.level, b.cell_y, b.cell_x, a.component_idx);
      });

      for (auto it = cell_components.begin(); it != cell_components.end(); )
      {
        auto& cc = *it;
        auto range_end = std::find_if(it, cell_components.end(),
                                      [cc](const CellComponent& v)
        {
          return cc.cell_x != v.cell_x || cc.cell_y != v.cell_y;
        });

        auto level = terrain_components_[cc.component_idx].level;

        auto cell_idx = level * num_cells_.x * num_cells_.y + cc.cell_y * num_cells_.x + cc.cell_x;
        terrain_cells_[cell_idx].first = static_cast<std::uint32_t>(component_mapping_.size());
        std::transform(it, range_end, std::back_inserter(component_mapping_),
                       [](const CellComponent& v)
        {
          return v.component_idx;
        });

        terrain_cells_[cell_idx].second = static_cast<std::uint32_t>(component_mapping_.size());
        it = range_end;
      }
    }
    
    resources::TerrainDefinition 
      TerrainMap::terrain_at(Vector2i position, std::int32_t level,
                             const resources::TerrainLibrary& terrain_lib) const
    {
      position.x = clamp(position.x, 0, track_size_.x - 1);
      position.y = clamp(position.y, 0, track_size_.y - 1);

      resources::TerrainDefinition result;
      auto cell = make_vector2(position.x >> cell_bits_, position.y >> cell_bits_);
      auto cell_idx = level *num_cells_.x * num_cells_.y + cell.y * num_cells_.x + cell.x;

      auto range = terrain_cells_[cell_idx];
      auto begin = component_mapping_.data() + range.first;
      auto end = component_mapping_.data() + range.second;

      std::int32_t alpha = 0;
      for (auto ptr = begin; ptr != end && alpha < 255; ++ptr)
      {
        auto& component = terrain_components_[*ptr];

        auto terrain_desc = detail::terrain_at(component, position);

        if (terrain_desc.terrain_id != 0 && terrain_desc.alpha != 0)
        {
          const auto& terrain = terrain_lib.terrain(terrain_desc.terrain_id);

          auto a = ((256 - alpha) * 255) >> 8;
          result = detail::interpolate_terrain(result, terrain, a);
          alpha += a;
        }        
      }

      if (alpha < 255 && base_terrain_ != 0)
      {
        const auto& base_terrain = terrain_lib.terrain(base_terrain_);        
        result = detail::interpolate_terrain(result, base_terrain, 255 - alpha);
      }

      return result;
    }
  }
}