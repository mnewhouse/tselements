/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "path_vertices.hpp"

#include <boost/container/small_vector.hpp>
#include <boost/iterator/counting_iterator.hpp>

#include <algorithm>
#include <array>
#include <cstdint>

namespace ts
{
  namespace scene
  {
    namespace detail
    {
      struct TexCoordAccumulator
      {
        float first = 0.0f;
        float second = 0.0f;
      };

      // This function simply multiplies the vertex positions by the given scale,
      // resulting in absolute texture coordinates.
      template <typename VertexIt>
      auto make_tiled_texture_coords(VertexIt vertex_it, VertexIt vertex_end,
                                     float scale, float z)
      {
        std::transform(vertex_it, vertex_end, vertex_it,
                       [=](auto vertex)
        {
          vertex.tex_coords.x = vertex.position.x / scale;
          vertex.tex_coords.y = vertex.position.y / scale;
          vertex.tex_coords.z = z;
          return vertex;
        });
      };

      template <typename VertexType, typename VertexFunc>
      auto generate_point_vertices(const PathVertexPoint& point,
                                   const PathVertexPoint& next_point,
                                   const resources::StrokeProperties& stroke_properties,
                                   resources::StrokeSegment::Side stroke_side,
                                   float texture_size, float texture_z,
                                   TexCoordAccumulator& tex_coord_x_accumulator,
                                   resources::BasicGeometry<VertexType>& output_model,
                                   VertexFunc&& vertex_func)
      {
        using resources::StrokeProperties;
        using resources::TrackPathNode;
        using resources::StrokeSegment;

        const TrackPathNode& first_node = *point.first;
        const TrackPathNode& second_node = *point.second;

        using boost::container::small_vector;
        using vertex_buffer = small_vector<PathVertex, 16>;
        using face_buffer = small_vector<resources::Face, 16>;

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

        auto texture_scale = 1.0f / texture_size;
        auto copy_vertices = [&](const auto& vertices, const auto& faces)
        {
          auto& output_vertices = output_model.vertices;
          auto& output_faces = output_model.faces;

          using std::begin;
          using std::end;

          auto vertex_index = static_cast<std::uint32_t>(output_vertices.size());
          std::transform(begin(vertices), end(vertices), std::back_inserter(output_vertices),
                         vertex_func);

          std::transform(begin(faces), end(faces), std::back_inserter(output_faces),
                         [=](resources::Face face)
          {
            face.first_index += vertex_index;
            face.second_index += vertex_index;
            face.third_index += vertex_index;
            return face;
          });
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

        auto make_vertex = [&](auto position, auto normal)
        {
          PathVertex vertex;
          vertex.position = make_2d(position);
          vertex.normal = normal;
          vertex.color = stroke_properties.color;
          return vertex;
        };

        vertex_buffer vertices;
        face_buffer faces;
        if (stroke_properties.type == StrokeProperties::Border)
        {
          // If we have a border, we need an inner and an outer ring, so to speak.
          // The distance from the inner to the outer ring is equal to the stroke width.
          auto outer = normal_3d * (width - stroke_offset);
          auto next_outer = next_normal_3d * (next_width - next_stroke_offset);

          auto inner = normal_3d * (width - stroke_offset - stroke_width);
          auto next_inner = next_normal_3d * (next_width - next_stroke_offset - next_stroke_width);

          auto add_vertices = [&](float d, std::uint32_t index)
          {
            auto p = point_3d + outer * d;
            auto idx = vertices.size();

            vertices.insert(vertices.end(),
            {
              make_vertex(point_3d + outer * d,
                          calculate_normal(normal_3d * d, stroke_properties.outer_normal)),

              make_vertex(next_point_3d + next_outer * d,
                          calculate_normal(next_normal_3d * d, stroke_properties.outer_normal)),

              make_vertex(point_3d + inner * d,
                          calculate_normal(normal_3d * d, stroke_properties.inner_normal)),

              make_vertex(next_point_3d + next_inner * d,
                          calculate_normal(next_normal_3d * d, stroke_properties.inner_normal))
            });

            faces.insert(faces.end(),
            {
              { 0 + index, 1 + index, 2 + index },
              { 1 + index, 2 + index, 3 + index }
            });
          };

          auto current_index = [&]() { return static_cast<std::uint32_t>(vertices.size()); };

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
          
          if (stroke_properties.texture_mode == StrokeProperties::Directional)
          {
            // If the texture mode is "directional", we need to adjust the texture
            // coordinates based on the direction of the path.
            // We need to get the distance from the first point to the second point, so
            // the textures are mapped in a uniform manner.

            auto add_tex_coords = [&](float x_accumulator, std::uint32_t vertex_index)
            {
              auto scale_inv = 1.0f / stroke_properties.texture_scale;
              auto v = &vertices[vertex_index];
              auto x_advance = ((distance(v[0].position, v[1].position) +
                                 distance(v[2].position, v[3].position)) * 0.5f) * scale_inv;
              auto new_accumulator = x_accumulator + x_advance;

              auto y_coord = width * scale_inv;
              auto next_y_coord = next_width * scale_inv;

              using std::begin;
              using std::end;
              const Vector3f coords[] =
              {
                { x_accumulator, -y_coord, texture_z },
                { new_accumulator, -next_y_coord, texture_z },
                { x_accumulator, y_coord, texture_z },
                { new_accumulator, next_y_coord, texture_z }
              };

              {
                std::size_t idx = vertex_index;
                for (auto coord : coords)
                {
                  vertices[idx].tex_coords = coord;
                  ++idx;
                }
              }
              
              return new_accumulator;
            };

            if (stroke_side != StrokeSegment::Second)
            {
              tex_coord_x_accumulator.first = add_tex_coords(tex_coord_x_accumulator.first,
                                                             first_side_vertex_index);

            }

            if (stroke_side != StrokeSegment::First)
            {
              tex_coord_x_accumulator.second = add_tex_coords(tex_coord_x_accumulator.second,
                                                              second_side_vertex_index);
            }
          }

          else
          {
            detail::make_tiled_texture_coords(vertices.begin(), vertices.end(),
                                              stroke_properties.texture_scale, texture_z);
          }
        }

        else
        {
          // Normal stroke, no border
          vertices =
          {
            make_vertex(point_3d + normal_3d * stroke_width,
            calculate_normal(normal_3d, stroke_properties.outer_normal)),

            make_vertex(next_point_3d + next_normal_3d * next_stroke_width,
                        calculate_normal(next_normal_3d, stroke_properties.outer_normal)),

            make_vertex(point_3d - normal_3d * stroke_width,
                        calculate_normal(-normal_3d, stroke_properties.outer_normal)),

            make_vertex(next_point_3d - next_normal_3d * next_stroke_width,
                        calculate_normal(-next_normal_3d, stroke_properties.outer_normal)),

            make_vertex(point_3d, calculate_normal(normal_3d, 0.0f)),

            make_vertex(next_point_3d, calculate_normal(next_normal_3d, 0.0f))
          };

          faces =
          {
            { 0, 1, 4 },
            { 2, 3, 4 },
            { 4, 5, 1 },
            { 4, 5, 3 }
          };

          if (stroke_properties.texture_mode == StrokeProperties::Directional)
          {
            // TODO
          }

          else
          {
            detail::make_tiled_texture_coords(vertices.begin(), vertices.end(),
                                              stroke_properties.texture_scale, texture_z);
          }
        }

        copy_vertices(vertices, faces);
      }

      template <typename VertexType, typename VertexTransform>
      auto generate_stroke_segment(const resources::TrackPath& path,
                                   const resources::StrokeSegment& segment,
                                   const resources::StrokeProperties& stroke_properties,
                                   const std::vector<PathVertexPoint>& points,
                                   float texture_size, float texture_z,
                                   resources::BasicGeometry<VertexType>& output_model,
                                   VertexTransform&& vertex_transform)
      {
        using Point = PathVertexPoint;
        auto first_node = path.nodes.begin();

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
          TexCoordAccumulator tex_coord_accumulator;

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
                                            texture_size, texture_z, tex_coord_accumulator,
                                            output_model, vertex_func);
          }

          else
          {
            detail::generate_point_vertices(start_point, *next_point_it, stroke_properties, segment.side,
                                            texture_size, texture_z, tex_coord_accumulator,
                                            output_model, vertex_func);

            ++point_it, ++next_point_it;

            for (; point_it != before_end; ++point_it, ++next_point_it)
            {
              detail::generate_point_vertices(*point_it, *next_point_it,
                                              stroke_properties, segment.side,
                                              texture_size, texture_z, tex_coord_accumulator,
                                              output_model, vertex_func);
            }

            detail::generate_point_vertices(*before_end, end_point, stroke_properties, segment.side,
                                            texture_size, texture_z, tex_coord_accumulator,
                                            output_model, vertex_func);
          }
        }
      }

      template <typename VertexType, typename VertexTransform>
      void generate_path_vertices(const resources::TrackPath& path,
                                  const resources::SegmentedStroke& stroke,
                                  const std::vector<PathVertexPoint>& points,                                  
                                  float texture_size, 
                                  float texture_z,
                                  resources::BasicGeometry<VertexType>& output_geometry,
                                  VertexTransform&& vertex_transform)
      {
        using resources::TrackPathNode;
        using resources::StrokeProperties;
        using resources::StrokeSegment;

        auto first_node = path.nodes.begin();
        if (!path.nodes.empty())
        {
          const auto& stroke_properties = stroke.properties;
          const auto& segments = stroke.segments;

          // For each segment, find matching range of vertex points and use that
          // to generate the vertices.
          for (StrokeSegment segment : stroke.segments)
          {
            auto max_index = static_cast<std::uint32_t>(path.nodes.size() - 1);

            // Sanitize the segment in case it's out of bounds.
            if (segment.end_index >= max_index)
            {
              segment.end_index = max_index;
              segment.end_time_point = 0.0f;
            }

            if (segment.start_index >= max_index)
            {
              segment.start_index = max_index;
              segment.start_time_point = 0.0f;
            }
            // If the path is closed, and the shortest route between the points
            // goes through the closing segment, generate two stroke segments.
            // One going from segment.first to path.begin, and another going from path.begin
            // to segment.second.
            auto first = segment.start_index + segment.start_time_point;
            auto second = segment.end_index + segment.end_time_point;
            if (second < first) std::swap(second, first);

            if (path.closed && second - first >= path.nodes.size() * 0.5f)
            {
              auto first_segment = segment;
              auto second_segment = segment;

              // Artificially generate two segments.
              if (segment.start_index + segment.start_time_point <
                  segment.end_index + segment.end_time_point)
              {
                std::swap(first_segment.start_index, second_segment.end_index);
                std::swap(first_segment.start_time_point, second_segment.end_time_point);
              }

              first_segment.end_index = max_index;
              first_segment.end_time_point = 0.0f;
              second_segment.start_index = 0;
              second_segment.start_time_point = 0.0f;

              generate_stroke_segment(path, first_segment, stroke_properties, points,
                                      texture_size, texture_z,
                                      output_model, vertex_func);
              generate_stroke_segment(path, second_segment, stroke_properties, points,
                                      texture_size, texture_z,
                                      output_geometry, vertex_transform);
            }

            else
            {
              generate_stroke_segment(path, segment, stroke_properties, points,
                                      texture_size, texture_z,
                                      output_geometry, vertex_transform);
            }
          }
        }
      }
    }

    template <typename VertexType, typename VertexTransform>
    void generate_path_vertices(const resources::TrackPath& path,
                                const resources::StrokeProperties& stroke_properties,
                                const std::vector<PathVertexPoint>& points,
                                float texture_size, float texture_z,
                                resources::BasicGeometry<VertexType>& geometry,
                                VertexTransform&& transform_vertex)
    {
      using resources::TrackPathNode;
      using resources::StrokeProperties;
      using resources::StrokeSegment;

      detail::TexCoordAccumulator tex_coord_x_accumulator;

      auto point_it = points.begin();
      if (point_it != points.end())
      {
        auto next_point_it = std::next(point_it);
        for (; next_point_it != points.end(); ++next_point_it, ++point_it)
        {
          detail::generate_point_vertices(*point_it, *next_point_it,
                                          stroke_properties, StrokeSegment::Both,
                                          texture_size, texture_z, tex_coord_x_accumulator,
                                          geometry, transform_vertex);
        }
      }
    }

    template <typename VertexType, typename VertexTransform>
    void generate_path_vertices(const resources::TrackPath& path,
                                const resources::SegmentedStroke& stroke,
                                const std::vector<PathVertexPoint>& points,
                                float texture_size, float texture_z,
                                resources::BasicGeometry<VertexType>& geometry,
                                VertexTransform&& transform_vertex)
    {
      if (stroke.properties.is_segmented)
      {
        detail::generate_path_vertices(path, stroke, points,
                                       texture_size, texture_z,
                                       geometry, transform_vertex);
      }

      else
      {
        generate_path_vertices(path, stroke.properties, points,
                               texture_size, texture_z,
                               geometry, transform_vertex);
      }
    }
  }
}
