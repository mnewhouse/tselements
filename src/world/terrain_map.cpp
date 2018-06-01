/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "terrain_map.hpp"

#include "resources/terrain_library.hpp"

#include "utility/interpolate.hpp"

#include <boost/geometry.hpp>

namespace ts
{
  namespace world
  {
    namespace map_components
    {
      Vector2i calculate_local_coords(const Pattern& pattern, Vector2i position)
      {
        auto coords = (position - pattern.position);

        const auto& t = pattern.transformation;

        auto result = make_vector2(
          (coords.x * t[1] + coords.y * t[0]) >> 16,
          (coords.y * t[1] - coords.x * t[0]) >> 16
        );

        result.x += pattern.pattern_rect.width >> 1;
        result.y += pattern.pattern_rect.height >> 1;

        return result;
      }

      TerrainDescriptor terrain_at(const Pattern& pattern, Vector2i position)
      {
        const auto& pat_rect = pattern.pattern_rect;
        auto coords = calculate_local_coords(pattern, position);

        if (coords.x >= 0 && coords.y >= 0 && coords.x < pat_rect.width && coords.y < pat_rect.height)
        {
          auto terrain_id = (*pattern.pattern)(coords.x + pat_rect.left, coords.y + pat_rect.top);
          return{ terrain_id, pattern.alpha };
        }

        return {};
      }

      TerrainDescriptor terrain_at(const Face& face, Vector2i position)
      {
        auto sign = [](auto a, auto b, auto p)
        {
          return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x) < 0;
        };

        const auto& v = face.vertices;
        
        if (sign(v[0], v[1], position) == sign(v[0], v[1], v[2]) &&
            sign(v[0], v[2], position) == sign(v[0], v[2], v[1]) &&
            sign(v[1], v[2], position) == sign(v[1], v[2], v[0]))
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
        const auto real_alpha = alpha / 255.0;

        resources::TerrainDefinition result;
        result.acceleration = interpolate_linearly(first.acceleration, second.acceleration, real_alpha);
        result.antislide = interpolate_linearly(first.antislide, second.antislide, real_alpha);
        result.braking = interpolate_linearly(first.braking, second.braking, real_alpha);
        result.cornering = interpolate_linearly(first.cornering, second.cornering, real_alpha);
        result.jump = interpolate_linearly(first.jump, second.jump, real_alpha);
        result.rolling_resistance = interpolate_linearly(first.rolling_resistance, second.rolling_resistance, real_alpha);
        result.roughness = interpolate_linearly(first.roughness, second.roughness, real_alpha);
        result.steering = interpolate_linearly(first.steering, second.steering, real_alpha);
        result.traction = interpolate_linearly(first.traction, second.traction, real_alpha);
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

    TerrainMap::box_type TerrainMap::bounding_box(const map_components::Pattern& pattern)
    {
      const auto& rect = pattern.pattern_rect;

      auto width = std::abs(rect.width * pattern.transformation[0]) + 
        std::abs(rect.height * pattern.transformation[1]);
      
      auto height = std::abs(rect.width * pattern.transformation[1]) +
        std::abs(rect.height * pattern.transformation[0]);

      width >>= 17;
      height >>= 17;

      return
      {
        { pattern.position.x - width, pattern.position.y - height },
        { pattern.position.x + width, pattern.position.y + height }
      };
    }

    TerrainMap::box_type TerrainMap::bounding_box(const map_components::Face& face)
    {
      // Find the min. and max. coordinates on both axes.
      auto minmax_x = std::minmax_element(face.vertices.begin(), face.vertices.end(),
                                          [](const auto& a, const auto& b)
      {
        return a.x < b.x;
      });

      auto minmax_y = std::minmax_element(face.vertices.begin(), face.vertices.end(), 
                                          [](const auto& a, const auto& b)
      {
        return a.y < b.y;
      });

      return
      { 
        { minmax_x.first->x, minmax_y.first->y },
        { minmax_x.second->x, minmax_y.second->y }
      };
    }

    TerrainMap::box_type TerrainMap::bounding_box(const map_components::Base& base)
    {
      return
      {
        { base.rect.left, base.rect.top },
        { base.rect.right(), base.rect.bottom() }
      };
    }

    TerrainMap::box_type TerrainMap::bounding_box(const TerrainMapComponent& component)
    {
      return boost::apply_visitor([](const auto& data)
      {
        return bounding_box(data);
      }, component.data);
    }

    resources::TerrainDefinition 
      TerrainMap::terrain_at(Vector2i position, std::int32_t level,
                             const resources::TerrainLibrary& terrain_lib) const
    {
      resources::TerrainDefinition result = {};
      auto ulevel = static_cast<std::size_t>(level);
      if (ulevel >= component_maps_.size())
      {
        return result;
      }

      const auto& tree = component_maps_[ulevel];
      terrain_buffer_.clear();
      
      auto query_it = tree.qbegin(boost::geometry::index::contains(point_type(position.x, position.y)));
      for (; query_it != tree.qend(); ++query_it)
      {
        const auto& component = query_it->second;

        InternalTerrainDescriptor terrain_desc;
        terrain_desc.z_index = component.z_index;
        static_cast<TerrainDescriptor&>(terrain_desc) = detail::terrain_at(component, position);

        if (terrain_desc.alpha != 0 && terrain_desc.terrain_id != 0)
        {
          terrain_buffer_.push_back(terrain_desc);
        }
      }      

      auto range_delim = std::partition(terrain_buffer_.begin(), terrain_buffer_.end(),
                                        [](const auto& desc)
      {
        return desc.alpha == 255;
      });

      auto base_terrain_it = std::max_element(terrain_buffer_.begin(), range_delim,
                                              [](const auto& a, const auto& b)
      {
        return a.z_index < b.z_index;
      });

      if (base_terrain_it != range_delim)
      {
        result = terrain_lib.terrain(base_terrain_it->terrain_id);

        auto min_z_index = base_terrain_it->z_index;
        terrain_buffer_.erase(std::remove_if(range_delim, terrain_buffer_.end(), 
                                             [=](const auto& elem)
        {
          return elem.z_index < min_z_index;
        }), terrain_buffer_.end());
      }

      std::sort(range_delim, terrain_buffer_.end(),
                [](const auto& a, const auto& b)
      {
        return a.z_index < b.z_index;
      });

      for (auto it = range_delim; it != terrain_buffer_.end(); ++it)
      {
        const auto& terrain = terrain_lib.terrain(it->terrain_id);
        result = detail::interpolate_terrain(result, terrain, it->alpha);
      }

      return result;
    }
  }
}