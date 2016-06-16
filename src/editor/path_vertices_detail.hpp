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
      template <typename PositionIt, typename TexCoordOut>
      auto make_tiled_texture_coords(PositionIt position_it, PositionIt position_end,
                                     float scale,
                                     TexCoordOut& tex_coord_out)
      {
        std::transform(position_it, position_end, tex_coord_out,
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
    }

    namespace detail
    {
      template <typename PathNodeIt, typename TexCoordAccumulator,
        typename VertexFunc, typename VertexOut, typename IndexOut>
      auto generate_point_vertices(const PathVertexPoint<PathNodeIt>& point,
                                   const PathVertexPoint<PathNodeIt>& next_point,
                                   const resources_3d::StrokeProperties& stroke_properties,
                                   resources_3d::StrokeSegment::Side stroke_side,
                                   TexCoordAccumulator& tex_coord_x_accumulator,
                                   VertexFunc& vertex_func, VertexOut& vertex_out,
                                   std::uint32_t& start_index, IndexOut& index_out)
      {
        using resources_3d::StrokeProperties;
        using resources_3d::TrackPathNode;
        using resources_3d::StrokeSegment;

        const TrackPathNode& first_node = *point.first;
        const TrackPathNode& second_node = *point.second;

        using boost::container::small_vector;
        using position_buffer = small_vector<Vector3f, 16>;
        using normal_buffer = small_vector<Vector3f, 16>;
        using tex_coord_buffer = small_vector<Vector2f, 16>;
        using index_buffer = small_vector<std::uint32_t, 32>;

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

        position_buffer positions;
        normal_buffer normals;
        tex_coord_buffer tex_coords;
        index_buffer indices;
        if (stroke_properties.type == StrokeProperties::Border)
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

          auto add_vertices = [&](float d, std::uint32_t index)
          {
            const Vector3f p[] =
            {
              point_3d + outer * d,
              next_point_3d + next_outer * d,
              point_3d + inner * d,
              next_point_3d + next_inner * d
            };

            const Vector3f n[] =
            {
              calculate_normal(normal_3d * d, stroke_properties.outer_normal),
              calculate_normal(next_normal_3d * d, stroke_properties.outer_normal),
              calculate_normal(normal_3d * d, stroke_properties.inner_normal),
              calculate_normal(next_normal_3d * d, stroke_properties.inner_normal),
            };

            const std::uint32_t i[] =
            {
              0 + index, 1 + index, 2 + index,
              1 + index, 2 + index, 3 + index
            };

            using std::begin;
            using std::end;

            positions.insert(positions.end(), begin(p), end(p));
            normals.insert(normals.end(), begin(n), end(n));
            indices.insert(indices.begin(), begin(i), end(i));
          };

          auto current_index = [&]() { return static_cast<std::uint32_t>(positions.size()); };

          auto first_side_vertex_index = current_index();
          auto second_side_vertex_index = current_index();

          // Add vertices for both sides
          if (stroke_side != StrokeSegment::Second)
          {
            add_vertices(-1.0f, first_side_vertex_index);
          }

          if (stroke_side != StrokeSegment::First)
          {
            second_side_vertex_index = current_index();
            add_vertices(1.0f, second_side_vertex_index);
          }

          auto first_side_bevel_vertex_index = current_index();
          auto second_side_bevel_vertex_index = current_index();

          if (bevel_width != 0.0f)
          {
            auto add_bevel_vertices = [&](float d, std::uint32_t side_index, std::uint32_t new_index)
            {
              const Vector3f p[] =
              {
                point_3d + bevel * d,
                next_point_3d + next_bevel * d,
              };

              const Vector3f n[] =
              {
                calculate_normal(normal_3d * d, bevel_strength),
                calculate_normal(next_normal_3d * d, bevel_strength),
              };

              using std::begin;
              using std::end;

              const std::uint32_t i[] =
              {
                side_index, side_index + 1, new_index,
                side_index + 1, new_index, new_index + 1
              };

              positions.insert(positions.end(), std::begin(p), std::end(p));
              normals.insert(normals.end(), std::begin(n), std::end(n));
              indices.insert(indices.end(), std::begin(i), std::end(i));
            };

            if (stroke_side != StrokeSegment::Second)
            {
              first_side_bevel_vertex_index = current_index();
              add_bevel_vertices(-1.0f, first_side_vertex_index, first_side_bevel_vertex_index);
            }

            if (stroke_side != StrokeSegment::First)
            {
              second_side_bevel_vertex_index = current_index();
              add_bevel_vertices(1.0f, second_side_vertex_index, second_side_bevel_vertex_index);
            }
          }

          if (stroke_properties.texture_mode == StrokeProperties::Directional)
          {
            // If the texture mode is "directional", we need to adjust the texture
            // coordinates based on the direction of the path.
            // We need to get the distance from the first point to the second point, so
            // the textures are mapped in a uniform manner.

            auto add_tex_coords = [&](float x_accumulator, std::uint32_t pos_index, std::uint32_t bevel_pos_index)
            {
              auto scale_inv = 1.0f / stroke_properties.texture_scale;
              auto p = &positions[pos_index];
              auto x_advance = ((distance(p[0], p[1]) + distance(p[2], p[3])) * 0.5f) * scale_inv;
              auto new_accumulator = x_accumulator + x_advance;

              auto y_coord = (width - bevel_width) * scale_inv;
              auto next_y_coord = (next_width - bevel_width) * scale_inv;

              using std::begin;
              using std::end;
              const Vector2f coords[] =
              {
                { x_accumulator, -y_coord },
                { new_accumulator, -next_y_coord },
                { x_accumulator, y_coord },
                { new_accumulator, next_y_coord }
              };

              std::copy(begin(coords), end(coords), tex_coords.begin() + pos_index);

              if (bevel_width != 0.0f)
              {
                auto bevel_y_coord = width * scale_inv;
                auto bevel_next_y_coord = next_width * scale_inv;

                const Vector2f bevel_coords[] =
                {
                  { x_accumulator, bevel_y_coord },
                  { new_accumulator, bevel_next_y_coord }
                };

                std::copy(begin(bevel_coords), end(bevel_coords), tex_coords.begin() + bevel_pos_index);
              }

              return new_accumulator;
            };

            if (stroke_side != StrokeSegment::Second)
            {
              tex_coord_x_accumulator.first = add_tex_coords(tex_coord_x_accumulator.first,
                                                             first_side_vertex_index,
                                                             first_side_bevel_vertex_index);

            }

            if (stroke_side != StrokeSegment::First)
            {
              tex_coord_x_accumulator.second = add_tex_coords(tex_coord_x_accumulator.second,
                                                             second_side_vertex_index,
                                                             second_side_bevel_vertex_index);
            }
          }

          else
          {
            detail::make_tiled_texture_coords(positions.begin(), positions.end(),
                                              stroke_properties.texture_scale,
                                              std::back_inserter(tex_coords));
          }
        }

        else
        {
          // Normal stroke, no border
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
          {
              0, 1, 4, 2, 3, 4, 4, 5, 1, 4, 5, 3
          };

          if (stroke_properties.texture_mode == StrokeProperties::Directional)
          {
            // copy_vertices(positions, indices, tex_coords);
          }

          else
          {
            detail::make_tiled_texture_coords(positions.begin(), positions.end(),
                                              stroke_properties.texture_scale,
                                              std::back_inserter(tex_coords));
          }
        }

        copy_vertices(positions, tex_coords, normals, indices);
      }
    }

    namespace detail
    {
      template <typename PathNodeIt, typename SegmentArray,
        typename VertexFunc, typename VertexOut, typename IndexOut>
      auto generate_path_vertices(const std::vector<PathVertexPoint<PathNodeIt>>& points,
                                  PathNodeIt first_node,
                                  const resources_3d::StrokeProperties& stroke_properties,
                                  const SegmentArray& segments,
                                  VertexFunc&& vertex_func, VertexOut vertex_out,
                                  std::uint32_t start_index, IndexOut index_out)
      {
        using Point = PathVertexPoint<PathNodeIt>;

        using resources_3d::TrackPathNode;
        using resources_3d::StrokeProperties;
        using resources_3d::StrokeSegment;

        // For each segment, find matching range of vertex points and use that
        // to generate the vertices.
        for (const StrokeSegment& segment : segments)
        {
          auto start = std::make_pair(first_node + segment.start_index, segment.start_time_point);
          auto end = std::make_pair(first_node + segment.end_index, segment.end_time_point);
          
          auto cmp = [](const auto& a, const auto& b)
          {
            return std::tie(a.first, a.time_point) < std::tie(b.first, b.second);
          };

          auto lower = std::lower_bound(points.begin(), points.end(), start, cmp);
          auto upper = std::lower_bound(points.begin(), points.end(), end, cmp);

          if (lower > upper)
          {
            std::swap(lower, upper);
            std::swap(start, end);
          }

          if (lower != points.begin()) --lower;

          if (lower != upper)
          {
            auto tex_coord_accumulator = std::make_pair(0.0f, 0.0f);

            auto num_points = static_cast<std::size_t>(std::distance(lower, upper));
            auto point_it = lower, next_point_it = std::next(point_it);

            auto interpolate = [=](const auto& a, const auto& b, float time_point, float time_span)
            {
              if (time_span == 0.0f) return a;

              return a + (b - a) * (time_point / time_span);
            };

            Point start_point;
            start_point.time_point = start.second;
            start_point.first = point_it->first;
            start_point.second = point_it->second;

            {
              auto time_point = (start.first - point_it->first) + (start.second - point_it->time_point);
              auto time_span = (next_point_it->first - point_it->first) +
                (next_point_it->time_point - point_it->time_point);
              start_point.normal = interpolate(point_it->normal, next_point_it->normal,
                                               time_point, time_span);
              start_point.point = interpolate(point_it->point, next_point_it->point,
                                              time_point, time_span);
            }

            // End point lies in the range prev(upper), upper
            auto after_end = upper;
            auto before_end = std::prev(upper);

            Point end_point;
            end_point.time_point = end.second;
            end_point.first = before_end->first;
            end_point.second = before_end->second;
            {
              auto time_point = (end.first - before_end->first) + (end.second - before_end->time_point);
              auto time_span = (after_end->first - before_end->first) +
                (after_end->time_point - before_end->time_point);

              end_point.normal = interpolate(before_end->normal, after_end->normal,
                                             time_point, time_span);
              end_point.point = interpolate(before_end->point, after_end->point,
                                              time_point, time_span);
            }          
            
            if (num_points == 1)
            {
              // There's only something between the start point and end point.
              detail::generate_point_vertices(start_point, end_point, stroke_properties, segment.side,
                                              tex_coord_accumulator,
                                              vertex_func, vertex_out, start_index, index_out);
            }

            else
            {
              detail::generate_point_vertices(start_point, *next_point_it, stroke_properties, segment.side,
                                              tex_coord_accumulator,
                                              vertex_func, vertex_out, start_index, index_out);

              ++point_it, ++next_point_it;

              for (; point_it != before_end; ++point_it, ++next_point_it)
              {
                detail::generate_point_vertices(*point_it, *next_point_it,
                                                stroke_properties, segment.side,
                                                tex_coord_accumulator,
                                                vertex_func, vertex_out, start_index, index_out);
              }

              detail::generate_point_vertices(*before_end, end_point, stroke_properties, segment.side,
                                              tex_coord_accumulator,
                                              vertex_func, vertex_out, start_index, index_out);
            }
          }
        }

        return std::make_pair(vertex_out, index_out);
      }
    }


    template <typename PathNodeIt, typename VertexFunc, typename VertexOut, typename IndexOut>
    auto generate_path_vertices(const std::vector<PathVertexPoint<PathNodeIt>>& points,
                                const resources_3d::StrokeProperties& stroke_properties,
                                VertexFunc&& vertex_func, VertexOut vertex_out,
                                std::uint32_t start_index, IndexOut index_out)
    {
      using resources_3d::TrackPathNode;
      using resources_3d::StrokeProperties;
      using resources_3d::StrokeSegment;

      auto tex_coord_x_accumulator = std::make_pair(0.0f, 0.0f);

      auto point_it = points.begin();
      if (point_it != points.end())
      {
        auto next_point_it = std::next(point_it);
        for (; next_point_it != points.end(); ++next_point_it, ++point_it)
        {
          detail::generate_point_vertices(*point_it, *next_point_it,
                                          stroke_properties, StrokeSegment::Both,
                                          tex_coord_x_accumulator,
                                          vertex_func, vertex_out, start_index, index_out);
        }
      }

      return std::make_pair(vertex_out, index_out);
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
                                PathNodeIt first_node,
                                const resources_3d::SegmentedStroke& stroke,
                                VertexFunc&& vertex_func, VertexOut vertex_out,
                                std::uint32_t start_index, IndexOut index_out)
    {
      if (stroke.properties.is_segmented)
      {
        return detail::generate_path_vertices(points, first_node, 
                                              stroke.properties, stroke.segments,
                                              vertex_func, vertex_out, start_index, index_out);
      }

      else
      {
        return generate_path_vertices(points, stroke.properties,
                                      vertex_func, vertex_out,
                                      start_index, index_out);
      }
    }
  }
}

#endif