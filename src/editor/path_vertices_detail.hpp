/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef PATH_VERTICES_DETAIL_HPP_843192834
#define PATH_VERTICES_DETAIL_HPP_843192834

#include "path_vertices.hpp"

#include <algorithm>
#include <cstdint>
#include <array>
#include <cmath>

#include <boost/container/small_vector.hpp>

namespace ts
{
  namespace scene_3d
  {
    namespace detail
    {
      template <typename NodeIt>
      auto make_path_vertex_point_at(NodeIt first_node, NodeIt second_node, float time_point)
      {
        PathVertexPoint<NodeIt> point;
        point.first = first_node;
        point.second = second_node;
        point.time_point = time_point;
        point.normal = resources_3d::path_normal_at(*first_node, *second_node, time_point);
        point.point = resources_3d::path_point_at(*first_node, *second_node, time_point);
        return point;
      }

      template <typename NodeIt>
      void divide_path_segment(std::vector<PathVertexPoint<NodeIt>>& vertex_points,
                               NodeIt first_node, NodeIt second_node,
                               float min_time_point, float max_time_point,
                               Vector2f first_point, Vector2f second_point,
                               Vector2f first_normal, Vector2f second_normal,
                               float tolerance)
      {
        if (std::abs(first_point.x - second_point.x) >= 1.0f ||
            std::abs(first_point.y - second_point.y) >= 1.0f)
        {
          auto time_interval = max_time_point - min_time_point;
          auto time_point = min_time_point + time_interval * 0.5f;
          auto halfway_point = resources_3d::path_point_at(*first_node, *second_node, time_point);
          auto halfway_normal = resources_3d::path_normal_at(*first_node, *second_node, time_point);

          // Use the square of the dot product of the normals to
          // determine how well they match. If 1.0 - dp² is larger than the tolerance,
          // proceed with the recursion.
          auto first_dp = std::abs(dot_product(first_normal, halfway_normal));
          if (1.0 - first_dp * first_dp > tolerance)
          {
            divide_path_segment(vertex_points, first_node, second_node,
                                min_time_point, time_point,
                                first_point, halfway_point,
                                first_normal, halfway_normal,
                                tolerance);
          }

          vertex_points.push_back(make_path_vertex_point_at(first_node, second_node, time_point));

          auto second_dp = std::abs(dot_product(halfway_normal, second_normal));
          if (1.0 - second_dp * second_dp > tolerance)
          {
            divide_path_segment(vertex_points, first_node, second_node,
                                time_point, max_time_point,
                                halfway_point, second_point,
                                halfway_normal, second_normal,
                                tolerance);
          }
        }
      }

      // This function simply returns the vertex positions multiplied by the given scale,
      // resulting in absolute texture coordinates.
      template <typename Positions, typename TexCoords>
      auto make_tiled_texture_coords(const Positions& positions, 
                                     float scale,
                                     TexCoords& tex_coords)
      {
        tex_coords.resize(positions.size());
        std::transform(positions.begin(), positions.end(), tex_coords.begin(),
                       [=](const auto& position) { return Vector2f(position.x, position.y) / scale; });
      }
    }

    // Precompute the points at which path vertices will be generated.
    // This is useful when a path has more than one stroke type, 
    // Lower tolerance values will give smoother results at the cost of more vertices.
    // 0.0 < tolerance <= 1.0, but it should not be too close to zero.
    // PathNodeIt must be a forward iterator that dereferences to resources_3d::TrackPathNode.
    template <typename PathNodeIt>
    auto compute_path_vertex_points(PathNodeIt node_it, PathNodeIt node_end, float tolerance,
                                    std::vector<PathVertexPoint<PathNodeIt>>& result)
    {
      using info_type = PathVertexPoint<PathNodeIt>;

      if (node_it != node_end)
      {
        auto next_node_it = std::next(node_it);
        for (; next_node_it != node_end; ++next_node_it, ++node_it)
        {
          // Divide the segment into 3 sub-segments to start with.
          // We could do just one sub-segment, but that way it won't work correctly for
          // some more complex paths.
          auto points =
          {
            detail::make_path_vertex_point_at(node_it, next_node_it, 0.0f),
            detail::make_path_vertex_point_at(node_it, next_node_it, 1.0f / 3),
            detail::make_path_vertex_point_at(node_it, next_node_it, 2.0f / 3),
            detail::make_path_vertex_point_at(node_it, next_node_it, 1.0f)
          };

          auto point_it = std::begin(points);
          auto next_point_it = std::next(point_it);
          for (; next_point_it != std::end(points); ++point_it, ++next_point_it)
          {
            result.push_back(*point_it);

            // Recursively divide the segment until it's smooth enough.
            detail::divide_path_segment(result, node_it, next_node_it,
                                        point_it->time_point, next_point_it->time_point,
                                        point_it->point, next_point_it->point,
                                        point_it->normal, next_point_it->normal,
                                        tolerance);
          }

          result.push_back(*point_it);       
        }
      }

      return result;
    }

    // Generate the vertices according to the vertex points previously generated by
    // compute_path_vertex_points. It writes the vertices and indices to the given
    // output iterators. 
    // The following syntax must be supported:
    //   *vertex_out++ = vertex_func(Vector3f(position), Vector2f(tex_coord), Vector3f(normal));
    // It also performs the operation 
    //   *index_out++ = std::size_t(start_index + index);
    // Note that the texture coordinates are absolute, meaning they may well have to be divided
    // by the texture size in order to be usable.
    template <typename PathNodeIt, typename VertexFunc, typename VertexOut, typename IndexOut>
    auto generate_path_vertices(const std::vector<PathVertexPoint<PathNodeIt>>& points,
                                const resources_3d::TrackPathStroke& stroke_properties,
                                VertexFunc&& vertex_func, VertexOut vertex_out,
                                std::uint32_t start_index, IndexOut index_out)
    {
      using resources_3d::TrackPathNode;
      using resources_3d::TrackPathStroke;

      auto tex_coord_x_accumulator = std::make_pair(0.0f, 0.0f);

      std::vector<Vector3f> positions; positions.reserve(16);
      std::vector<std::uint32_t> indices; indices.reserve(32);
      std::vector<Vector3f> normals; normals.reserve(16);
      std::vector<Vector2f> tex_coords; tex_coords.reserve(16);

      auto point_it = points.begin();
      if (point_it != points.end())
      {
        auto next_point_it = std::next(point_it);
        for (; next_point_it != points.end(); ++next_point_it, ++point_it)
        {
          using Point = PathVertexPoint<PathNodeIt>;

          const Point& point = *point_it;
          const Point& next_point = *next_point_it;

          using resources_3d::TrackPathNode;
          const TrackPathNode& first_node = *point.first;
          const TrackPathNode& second_node = *point.second;

          // Interpolate the width for this point
          auto width = second_node.width * point.time_point +
            first_node.width * (1.0f - point.time_point);

          // ...and the next point
          auto next_width = next_point.second->width * next_point.time_point +
            next_point.first->width * (1.0f - next_point.time_point);

          // Multiply the widths by half, because we'll be using them as distance from the center.
          width *= 0.5f;
          next_width *= 0.5f;

          auto stroke_width = stroke_properties.width;
          auto next_stroke_width = stroke_width;
          auto stroke_offset = stroke_properties.offset;
          auto next_stroke_offset = stroke_offset;
          
          if (stroke_properties.use_relative_size)
          {
            stroke_width *= width;
            next_stroke_width *= next_width;
            stroke_offset *= width;
            next_stroke_offset *= next_width;
          }

          auto index_func = [=](std::uint32_t index) { return index + start_index; };
          auto copy_vertices = [&](const auto& positions, const auto& texture_coords, 
                                   const auto& normals, const auto& indices)
          {
            for (std::size_t idx = 0; idx != positions.size(); ++idx)
            {
              Vector3f vec3 = { positions[idx].x, positions[idx].y, 0.0f };
              *vertex_out++ = vertex_func(vec3, texture_coords[idx], normals[idx]);
            }

            index_out = std::transform(std::begin(indices), std::end(indices),
                                       index_out, index_func);

            start_index += static_cast<std::uint32_t>(positions.size());
          };

          Vector3f normal_3d = { point.normal.x, point.normal.y, 0.0f };
          Vector3f next_normal_3d = { next_point.normal.x, next_point.normal.y, 0.0f };

          Vector3f point_3d = { point.point.x, point.point.y, 0.0f };
          Vector3f next_point_3d = { next_point.point.x, next_point.point.y, 0.0f };

          auto calculate_normal = [](auto normal, float factor)
          {
            // Interpolate the normal between the given normal and the "straight up" vector (0, 0, 1).
            return normal * factor + Vector3f(0.0f, 0.0f, 1.0f) * (1.0f - std::abs(factor));
          };

          if (stroke_properties.type == resources_3d::TrackPathStroke::Border)
          {
            auto bevel_width = stroke_properties.bevel_width;
            auto bevel_strength = stroke_properties.bevel_strength;

            // If we have a border, we need an inner and an outer ring, so to speak.
            // The distance from the inner to the outer ring is equal to the stroke width.
            auto outer = normal_3d * (width - bevel_width - stroke_offset);
            auto next_outer = next_normal_3d * (next_width - bevel_width - next_stroke_offset);

            auto inner = normal_3d * (width - stroke_offset - bevel_width - stroke_width);
            auto next_inner = next_normal_3d * (next_width - bevel_width - 
                                                next_stroke_offset - next_stroke_width);

            auto bevel = outer + normal_3d * bevel_width;
            auto next_bevel = next_outer + next_normal_3d * bevel_width;

            positions =
            {
              point_3d + outer,
              next_point_3d + next_outer,
              point_3d + inner,
              next_point_3d + next_inner,

              // Other side
              point_3d - outer,
              next_point_3d - next_outer,
              point_3d - inner,
              next_point_3d - next_inner
            };

            indices =
            {
              0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7
            };

            normals =
            {
              calculate_normal(normal_3d, stroke_properties.outer_normal),
              calculate_normal(next_normal_3d, stroke_properties.outer_normal),
              calculate_normal(normal_3d, stroke_properties.inner_normal),
              calculate_normal(next_normal_3d, stroke_properties.inner_normal),

              calculate_normal(-normal_3d, stroke_properties.outer_normal),
              calculate_normal(-next_normal_3d, stroke_properties.outer_normal),
              calculate_normal(-normal_3d, stroke_properties.inner_normal),
              calculate_normal(-next_normal_3d, stroke_properties.inner_normal),
            };

            if (bevel_width != 0.0f)
            {
              positions.insert(positions.end(),
              {
                point_3d + bevel,
                next_point_3d + next_bevel,
                point_3d - bevel,
                next_point_3d - next_bevel
              });

              normals.insert(normals.end(),
              {
                calculate_normal(normal_3d, bevel_strength),
                calculate_normal(next_normal_3d, bevel_strength),
                calculate_normal(-normal_3d, bevel_strength),
                calculate_normal(-next_normal_3d, bevel_strength)
              });

              indices.insert(indices.end(), 
              {
                0, 1, 8, 8, 9, 1, 4, 5, 10, 10, 11, 5
              });
            }

            if (stroke_properties.texture_mode == TrackPathStroke::Directional)
            {
              // If the texture mode is "directional", we need to adjust the texture
              // coordinates based on the direction of the path.
              // We need to get the distance from the first point to the second point, so
              // the textures are mapped in a uniform manner.

              auto point_distance = std::make_pair(
                (distance(positions[0], positions[1]) + distance(positions[2], positions[3])) * 0.5f,
                (distance(positions[4], positions[5]) + distance(positions[6], positions[7])) * 0.5f
                );

              auto new_accumulator = std::make_pair(
                tex_coord_x_accumulator.first + point_distance.first / stroke_properties.texture_scale,
                tex_coord_x_accumulator.second + point_distance.second / stroke_properties.texture_scale
              );              

              auto y_coord = (width - bevel_width) / stroke_properties.texture_scale;
              auto next_y_coord = (next_width - bevel_width) / stroke_properties.texture_scale;

              tex_coords =
              {
                { tex_coord_x_accumulator.first, -y_coord },
                { new_accumulator.first, -next_y_coord },
                { tex_coord_x_accumulator.first, y_coord },
                { new_accumulator.first, next_y_coord },

                { tex_coord_x_accumulator.second, -y_coord },
                { new_accumulator.second, -next_y_coord },
                { tex_coord_x_accumulator.second, y_coord },
                { new_accumulator.second, next_y_coord },
              };

              if (bevel_width != 0.0f)
              {
                auto bevel_y_coord = width / stroke_properties.texture_scale;
                auto bevel_next_y_coord = next_width / stroke_properties.texture_scale;

                tex_coords.insert(tex_coords.end(),
                {
                  { tex_coord_x_accumulator.first, bevel_y_coord },
                  { new_accumulator.first, bevel_next_y_coord },
                  { tex_coord_x_accumulator.second, -bevel_y_coord },
                  { new_accumulator.second, -bevel_next_y_coord }
                });
              }

              tex_coord_x_accumulator = new_accumulator;

              copy_vertices(positions, tex_coords, normals, indices);
            }

            else
            {
              detail::make_tiled_texture_coords(positions, 
                                                stroke_properties.texture_scale,
                                                tex_coords);
              
              copy_vertices(positions, tex_coords, normals, indices);
            }
          }

          else
          {
            positions =
            {
              point_3d + normal_3d * stroke_width,
              next_point_3d + next_normal_3d * next_stroke_width,
              point_3d - normal_3d * stroke_width,
              next_point_3d - next_normal_3d * next_stroke_width,
              point_3d,
              next_point_3d
            };

            normals =
            {
              calculate_normal(normal_3d, stroke_properties.outer_normal),
              calculate_normal(next_normal_3d, stroke_properties.outer_normal),
              calculate_normal(-normal_3d, stroke_properties.outer_normal),
              calculate_normal(-next_normal_3d, stroke_properties.outer_normal),
              calculate_normal(normal_3d, 0.0f),
              calculate_normal(next_normal_3d, 0.0f)
            };

            indices =
            { {
              0, 1, 4, 2, 3, 4, 4, 5, 1, 4, 5, 3
            } };

            if (stroke_properties.texture_mode == TrackPathStroke::Directional)
            {
              // copy_vertices(positions, indices, tex_coords);
            }

            else
            {
              detail::make_tiled_texture_coords(positions,
                                                stroke_properties.texture_scale,
                                                tex_coords);

              copy_vertices(positions, tex_coords, normals, indices);
            }            
          }
        }
      }

      return std::make_pair(vertex_out, index_out);
    }
  }
}

#endif