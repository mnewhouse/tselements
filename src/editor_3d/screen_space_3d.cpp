/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "screen_space_3d.hpp"

#include "resources_3d/elevation_map_3d.hpp"

#include <cmath>
#include <numeric>

namespace ts
{
  namespace editor3d
  {
    Vector2f relative_screen_position(Vector2i screen_pos, Vector2i screen_size)
    {
      auto pos = vector2_cast<float>(screen_pos) / vector2_cast<float>(screen_size);
      return make_vector2((pos.x - 0.5f) * 2.0f, (pos.y - 0.5f) * -2.0f);
    }

    Vector2f relative_screen_position(Vector2i screen_pos, IntRect screen_rect)
    {
      screen_pos.x -= screen_rect.left;
      screen_pos.y -= screen_rect.top;

      return relative_screen_position(screen_pos, make_vector2(screen_rect.width, screen_rect.height));
    }

    namespace detail
    {
      static auto find_screen_z(Vector2f pos, const glm::mat4& mat, float world_z)
      {
        auto i = mat[0].z;
        auto j = mat[1].z;
        auto k = mat[2].z;
        auto l = mat[3].z;
        auto m = mat[0].w;
        auto n = mat[1].w;
        auto o = mat[2].w;
        auto p = mat[3].w;

        auto d = (k - o * world_z);
        return ((-i * pos.x) - (j * pos.y) - l + (m * world_z * pos.x) + (n * world_z * pos.y) + (p * world_z)) / d;          
      }

      boost::optional<Vector3f> triangle_ray_intersection(const Vector3f& t1, const Vector3f& t2, const Vector3f& t3,
                                                          const Vector3f& ray_start, const Vector3f& ray_end)
      {
        auto u = t2 - t1;
        auto v = t3 - t1;

        auto plane_normal = cross_product(u, v);
        if (plane_normal == Vector3f{}) return boost::none;

        auto ray_direction = ray_end - ray_start;
        auto ray_offset = ray_start - t1;

        auto a = -dot_product(plane_normal, ray_offset);
        auto b = dot_product(plane_normal, ray_direction);
        if (std::abs(b) < 0.0001f) return boost::none;

        auto r = a / b;
        if (r >= 0.0f && r <= 1.0f)
        {
          Vector3f intersect_point = ray_start + ray_direction * r;

          auto uu = dot_product(u, u);
          auto uv = dot_product(u, v);
          auto vv = dot_product(v, v);
          auto w = intersect_point - t1;
          auto wu = dot_product(w, u);
          auto wv = dot_product(w, v);
          auto d = 1.0f / (uv * uv - uu * vv);

          auto s = (uv * wv - vv * wu) * d;
          if (s >= 0.0f && s <= 1.0f)
          {
            auto t = (uv * wu - uu * wv) * d;
            if (t >= 0.0f && s + t <= 1.0f)
            {
              return intersect_point;
            }
          }
        }

        return boost::none;
      }
    }

    boost::optional<Vector3f> ground_position_at(Vector2i screen_pos, IntRect view_port,
                                                 const glm::mat4& inverse_projected_view,
                                                 const resources3d::ElevationMap& elevation_map)
    {
      return ground_position_at(relative_screen_position(screen_pos, view_port), 
                                inverse_projected_view, elevation_map);
    }

    boost::optional<Vector3f> ground_position_at(Vector2f screen_pos, const glm::mat4& inverse_projected_view,
                                                 const resources3d::ElevationMap& elevation_map)
    {
      // Calculate the lowest and highest possible screen z coordinate, using the boundaries of the elevation map
      // as a guideline.
      auto min_z = detail::find_screen_z(screen_pos, inverse_projected_view, elevation_map.lowest_elevation());
      auto max_z = detail::find_screen_z(screen_pos, inverse_projected_view, elevation_map.highest_elevation());
      
      auto pos_a = inverse_projected_view * glm::vec4(screen_pos.x, screen_pos.y, min_z, 1.0f);
      auto pos_b = inverse_projected_view * glm::vec4(screen_pos.x, screen_pos.y, max_z, 1.0f);

      pos_a /= pos_a.w;
      pos_b /= pos_b.w;      

      auto ray_start = make_vector3(pos_a.x, pos_a.y, pos_a.z);
      auto ray_end = make_vector3(pos_b.x, pos_b.y, pos_b.z);

      auto grid_size = elevation_map.grid_size();

      auto cell_size = static_cast<float>(elevation_map.cell_size());
      auto inverse_cell_size = 1.0f / cell_size;

      auto cell_offset = elevation_map.cell_offset();

      auto min_pos = make_vector2(pos_a.x, pos_a.y);
      auto max_pos = make_vector2(pos_b.x, pos_b.y);

      auto compute_cell = [=](auto pos)
      {
        return make_vector2(static_cast<std::int32_t>(std::floor(pos.x * inverse_cell_size)),
                            static_cast<std::int32_t>(std::floor(pos.y * inverse_cell_size)));
      };

      auto test_cell = [&](Vector2i cell) -> boost::optional<Vector3f>
      {
        const Vector2i corners[] =
        {
          cell, // top left
          make_vector2(cell.x + 1, cell.y), // top right
          make_vector2(cell.x, cell.y + 1), // bottom left
          make_vector2(cell.x + 1, cell.y + 1) // bottom right
        };

        float elevation[4];
        for (int i = 0; i != 4; ++i)
        {
          elevation[i] = elevation_map.elevation_at(corners[i]);
        }

        auto center_elevation = std::accumulate(elevation, elevation + 4, 0.0f) * 0.25f;

        const Vector3f points[] =
        {
          make_3d(corners[0] * cell_size, elevation[0]),
          make_3d(corners[1] * cell_size, elevation[1]),
          make_3d(corners[2] * cell_size, elevation[2]),
          make_3d(corners[3] * cell_size, elevation[3]),
          make_3d((cell + 0.5f) * cell_size, center_elevation)
        };

        // Blast the ray that represents the screen pixel and test if it intersects with any
        // of the four triangles the cell consists of.
        auto test_triangle = [&](const Vector3f& a, const Vector3f& b, const Vector3f& c)
          -> boost::optional<Vector3f>
        {
          return detail::triangle_ray_intersection(a, b, c, ray_start, ray_end);
        };

        const int indices[][3] =
        {
          { 0, 1, 4 },
          { 0, 2, 4 },
          { 1, 3, 4 },
          { 2, 3, 4 }
        };

        for (const auto& t : indices)
        {
          auto a = t[0], b = t[1], c = t[2];
          if (auto result = test_triangle(points[a], points[b], points[c]))
          {
            return result;
          }
        }

        return boost::none;
      };

      auto min_cell = compute_cell(min_pos);
      auto max_cell = compute_cell(max_pos);

      IntRect cell_bounds(-cell_offset, grid_size - 2);

      min_cell.x = utility::clamp(min_cell.x, cell_bounds.left, cell_bounds.right());
      min_cell.y = utility::clamp(min_cell.y, cell_bounds.top, cell_bounds.bottom());
      max_cell.x = utility::clamp(max_cell.x, cell_bounds.left, cell_bounds.right());
      max_cell.y = utility::clamp(max_cell.y, cell_bounds.top, cell_bounds.bottom());

      auto x_limits = std::minmax(min_cell.x, max_cell.x);
      auto y_limits = std::minmax(min_cell.y, max_cell.y);

      
      boost::optional<Vector3f> result;
      boost::optional<float> best_screen_z;

      for (auto y = y_limits.first; y <= y_limits.second; ++y)
      {
        for (auto x = x_limits.first; x <= x_limits.second; ++x)
        {
          if (auto pos = test_cell({ x, y }))
          {
            auto screen_z = detail::find_screen_z(make_2d(*pos), inverse_projected_view, pos->z);
            if (!result || screen_z < *best_screen_z)
            {
              result = pos;
              best_screen_z.emplace(screen_z);
            }            
          }
        }
      }

      return result;
    }

    Vector2f relative_screen_position_at(Vector3f world_position, const glm::mat4& projected_view)
    {
      auto screen_pos = projected_view * glm::vec4(world_position.x, world_position.y, world_position.z, 1.0f);

      return make_vector2(screen_pos.x / screen_pos.w, screen_pos.y / screen_pos.w);
    }

    Vector2f screen_position_at(Vector3f world_position, const glm::mat4& projected_view, Vector2i screen_size)
    {
      return screen_position_at(world_position, projected_view, IntRect(0, 0, screen_size.x, screen_size.y));
    }

    Vector2f screen_position_at(Vector3f world_position, const glm::mat4& projected_view, IntRect view_port)
    {
      auto relative_pos = relative_screen_position_at(world_position, projected_view);
      
      relative_pos.x = relative_pos.x * 0.5f + 0.5f;
      relative_pos.y = -relative_pos.y * 0.5f + 0.5f;

      return make_vector2(view_port.left + relative_pos.x * view_port.width,
                          view_port.top + relative_pos.y * view_port.height);
    }
  }
}