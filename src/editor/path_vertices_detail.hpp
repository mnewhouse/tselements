/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef PATH_VERTICES_DETAIL_HPP_843192834
#define PATH_VERTICES_DETAIL_HPP_843192834

#include "path_vertices.hpp"

#include <boost/container/small_vector.hpp>
#include <boost/iterator/counting_iterator.hpp>

#include <algorithm>
#include <array>
#include <cstdint>

namespace ts
{
  namespace scene_3d
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
                                   const resources_3d::StrokeProperties& stroke_properties,
                                   resources_3d::StrokeSegment::Side stroke_side,
                                   float texture_size, float texture_z,
                                   TexCoordAccumulator& tex_coord_x_accumulator,
                                   resources_3d::BasicModel<VertexType>& output_model,
                                   VertexFunc&& vertex_func)
      {
        using resources_3d::StrokeProperties;
        using resources_3d::TrackPathNode;
        using resources_3d::StrokeSegment;

        const TrackPathNode& first_node = *point.first;
        const TrackPathNode& second_node = *point.second;

        using boost::container::small_vector;
        using vertex_buffer = small_vector<PathVertex, 16>;
        using face_buffer = small_vector<resources_3d::ModelFace, 16>;

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
                         [=](resources_3d::ModelFace face)
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

          auto first_side_bevel_vertex_index = current_index();
          auto second_side_bevel_vertex_index = current_index();

          if (bevel_width != 0.0f)
          {
            auto add_bevel_vertices = [&](float d, std::uint32_t side_index, std::uint32_t new_index)
            {
              vertices.insert(vertices.end(),
              {
                make_vertex(point_3d + bevel * d,
                calculate_normal(normal_3d * d, bevel_strength)),
                              make_vertex(next_point_3d + next_bevel * d,
                                          calculate_normal(next_normal_3d * d, bevel_strength))
              });


              faces.insert(faces.end(),
              {
                { side_index, side_index + 1, new_index },
                { side_index + 1, new_index, new_index + 1 }
              });
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

            auto add_tex_coords = [&](float x_accumulator, std::uint32_t vertex_index, std::uint32_t bevel_pos_index)
            {
              auto scale_inv = 1.0f / stroke_properties.texture_scale;
              auto v = &vertices[vertex_index];
              auto x_advance = ((distance(v[0].position, v[1].position) +
                                 distance(v[2].position, v[3].position)) * 0.5f) * scale_inv;
              auto new_accumulator = x_accumulator + x_advance;

              auto y_coord = (width - bevel_width) * scale_inv;
              auto next_y_coord = (next_width - bevel_width) * scale_inv;

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

              if (bevel_width != 0.0f)
              {
                auto bevel_y_coord = width * scale_inv;
                auto bevel_next_y_coord = next_width * scale_inv;

                const Vector3f bevel_coords[] =
                {
                  { x_accumulator, bevel_y_coord, texture_z },
                  { new_accumulator, bevel_next_y_coord, texture_z }
                };

                std::size_t idx = vertex_index;
                for (auto coord : bevel_coords)
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

      template <typename VertexType, typename VertexFunc>
      auto generate_stroke_segment(const resources_3d::TrackPath& path,
                                   const resources_3d::StrokeSegment& segment,
                                   const resources_3d::StrokeProperties& stroke_properties,
                                   const std::vector<PathVertexPoint>& points,
                                   float texture_size, float texture_z,
                                   resources_3d::BasicModel<VertexType>& output_model,
                                   VertexFunc&& vertex_func)
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

      template <typename VertexType, typename VertexFunc>
      void generate_path_vertices(const resources_3d::TrackPath& path,
                                  const resources_3d::SegmentedStroke& stroke,
                                  const std::vector<PathVertexPoint>& points,                                  
                                  float texture_size, 
                                  float texture_z,
                                  resources_3d::BasicModel<VertexType>& output_model,
                                  VertexFunc&& vertex_func)
      {
        using resources_3d::TrackPathNode;
        using resources_3d::StrokeProperties;
        using resources_3d::StrokeSegment;

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
                                      output_model, vertex_func);
            }

            else
            {
              generate_stroke_segment(path, segment, stroke_properties, points,
                                      texture_size, texture_z,
                                      output_model, vertex_func);
            }
          }
        }
      }
    }

    namespace detail
    {
      void find_cell_edge_intersections(Vector2f a, Vector2f b,
                                        std::uint32_t edge_index,
                                        const PathCellAlignment& cell_alignment,
                                        std::vector<EdgeIntersection>& edge_intersections);

      void add_cell_corner_intersections(const std::array<Vector2f, 4>& points,
                                         const std::array<std::pair<int, int>, 4>& edges,
                                         const PathCellAlignment& cell_alignment,
                                         std::vector<EdgeIntersection>& edge_intersections);

      void add_quad_corner_intersections(const std::array<Vector2f, 4>& points,
                                         const PathCellAlignment& cell_alignment,
                                         std::vector<EdgeIntersection>& edge_intersections);

      template <typename VertexType, typename VertexFunc>
      void add_edge_intersection_vertices(std::vector<EdgeIntersection>& edge_intersections,
                                          std::size_t start_index,
                                          resources_3d::BasicModel<VertexType>& output_model,
                                          VertexFunc&& make_vertex)
      {
        auto& output_vertices = output_model.vertices;
        auto vertex_index = static_cast<std::uint32_t>(output_vertices.size());

#if 0 // DEBUG VERTICES
        auto o = -0.8f;
        const Vector2f offsets[4] = { { -o, -o }, { -o, o }, { o, -o}, { o, o } };

        for (auto it = edge_intersections.begin() + start_index; it != edge_intersections.end(); ++it)
        {
          for (auto o : offsets)
          {
            output_model.vertices.push_back(make_vertex(it->intersection + o));
            output_model.vertices.back().color = { 255, 0, 0, 80 };
          }

          output_model.faces.push_back({ vertex_index, vertex_index + 1, vertex_index + 2 });
          output_model.faces.push_back({ vertex_index + 1, vertex_index + 2, vertex_index + 3 });

          vertex_index += 4;
        }
#else

        for (auto it = edge_intersections.begin() + start_index; it != edge_intersections.end(); ++it)
        {
          it->vertex_index = vertex_index;
          output_model.vertices.push_back(make_vertex(it->intersection));
          ++vertex_index;
        }
#endif
      }

      template <typename VertexType, typename VertexFunc>
      void add_edge_intersection_faces(const std::array<Vector2f, 4>& quad_points,
                                       const std::array<std::pair<int, int>, 4>& edges,
                                       const PathCellAlignment& cell_alignment,
                                       const std::vector<EdgeIntersection>& intersection_buffer,
                                       resources_3d::BasicModel<VertexType>& output_model,
                                       VertexFunc&& make_vertex)
      {
        const std::array<std::pair<Vector2f, Vector2f>, 4> edge_points =
        {{
          { quad_points[edges[0].first], quad_points[edges[0].second] },
          { quad_points[edges[1].first], quad_points[edges[1].second] },
          { quad_points[edges[2].first], quad_points[edges[2].second] },
          { quad_points[edges[3].first], quad_points[edges[3].second] },
        }};

        auto cell_size = cell_alignment.cell_size;
        auto real_cell_size = static_cast<float>(cell_size);

        auto& output_vertices = output_model.vertices;
        auto& output_faces = output_model.faces;


        enum CellEdge
        {
          Left, Top, Right, Bottom, Diagonal
        };

        auto find_adjacent_quad_edges = [=](const EdgeIntersection& e)
        {
          boost::container::small_vector<std::uint32_t, 4> result;

          switch (e.type)
          {
          case EdgeIntersection::QuadCorner:
          {
            std::uint32_t edge_index = 0;
            for (auto edge : edges)
            {
              if (edge.first == e.edge_index || edge.second == e.edge_index)
              {
                result.push_back(edge_index);
              }

              ++edge_index;
            }

            break;
          }

          default:
            result.push_back(e.edge_index);
          }

          return result;
        };

        auto find_adjacent_cell_edges = [](const EdgeIntersection& e)
          -> boost::container::small_vector<CellEdge, 4>
        {
          switch (e.type)
          {
          case EdgeIntersection::CellCorner:
            switch (e.edge_index)
            {
            case EdgeIntersection::TopLeft: return{ Left, Top, Diagonal };
            case EdgeIntersection::BottomLeft: return{ Left, Bottom };
            case EdgeIntersection::TopRight: return{ Right, Top };
            case EdgeIntersection::BottomRight: return{ Right, Bottom, Diagonal };
            default: return{};
            }

          case EdgeIntersection::Horizontal:
            if (e.cell_offset == 0) { return{ Top }; }
            else return{ Bottom };

          case EdgeIntersection::Vertical:
            if (e.cell_offset == 0) { return{ Left }; }
            else { return{ Right }; }

          case EdgeIntersection::Diagonal:
            return{ Diagonal };

          default:
            return{};
          }
        };

        // Now, loop through all intersecting grid cells, starting on the left, moving over to the right,
        // and going from top to bottom for each column.
        for (auto it = intersection_buffer.begin(); it != intersection_buffer.end(); )
        {
          const auto& edge_intersection = *it;
          auto cell = edge_intersection.cell;

          // Find the first intersection entry that does not match up with our current grid cell.
          auto range_end = std::find_if(std::next(it), intersection_buffer.end(),
                                        [=](const EdgeIntersection& e)
          {
            return e.cell != cell;
          });

          auto top_left = vector2_cast<float>(cell) * real_cell_size;        

          Vector2f cell_corners[4] =
          {
            top_left, top_left, top_left, top_left
          };

          // 0 = top left
          cell_corners[1].x += real_cell_size; // Top right
          cell_corners[2].y += real_cell_size; // Bottom left
          cell_corners[3] += real_cell_size; // Bottom right

          boost::optional<std::uint32_t> cell_corner_vertices[4] = {};
          
          // Grid cell consists of two triangles. For each triangle, add the faces that we need to.
          auto make_triangle_faces = [&](int horizontal_edge, int vertical_edge, 
                                         std::uint32_t excluded_corner, std::uint32_t triangle_id)
          {
            boost::container::small_vector<EdgeIntersection, 16> intersection_buffer;
            std::copy_if(it, range_end, std::back_inserter(intersection_buffer),
                         [=](const EdgeIntersection& e)
            {
              switch (e.type)
              {
              case EdgeIntersection::Diagonal: return true;
              case EdgeIntersection::Horizontal: return e.cell_offset == horizontal_edge;
              case EdgeIntersection::Vertical: return e.cell_offset == vertical_edge;
              case EdgeIntersection::CellCorner: return e.edge_index != excluded_corner;
              case EdgeIntersection::QuadCorner: return e.cell_offset == triangle_id;
              default: return false;                
              }
            });

            auto buffer_begin = intersection_buffer.begin();
            auto buffer_end = intersection_buffer.end();

            // Establish a "base" point that has two adjacent edges and one opposing edge.
            // Start with the triangle these edges represent, then keep expanding by creating
            // the triangles adjacent to the previous ones.

            auto find_adjacent_edge_intersections = [=](const EdgeIntersection& e, auto begin, auto end)
            {
              auto quad_edges = find_adjacent_quad_edges(e);
              auto cell_edges = find_adjacent_cell_edges(e);

              using iterator_type = decltype(begin);
              boost::container::small_vector<iterator_type, 16> result;
              for (auto it = begin; it != end; ++it)
              {
                const auto& other = *it;
                if (&e == &other) continue;

                auto is_adjacent = [&]()
                {
                  for (auto edge : find_adjacent_cell_edges(other))
                  {
                    if (std::find(cell_edges.begin(), cell_edges.end(), edge) != cell_edges.end()) return true;
                  }

                  for (auto edge : find_adjacent_quad_edges(other))
                  {
                    if (std::find(quad_edges.begin(), quad_edges.end(), edge) != quad_edges.end()) return true;
                  }

                  return false;
                };

                if (is_adjacent())
                {
                  result.push_back(it);
                }
              }

              return result;
            };

            auto find_base_point = [&]()
            {
              using result_type = decltype(find_adjacent_edge_intersections(*buffer_begin, buffer_begin, buffer_end));
              for (auto it = buffer_begin; it != buffer_end; ++it)
              {
                auto result = find_adjacent_edge_intersections(*it, buffer_begin, buffer_end);
                if (result.size() == 2) return std::make_pair(it, result);
              }

              return std::make_pair(buffer_end, result_type());
            };

            auto buffer_size = intersection_buffer.size();
            if (buffer_size >= 3)
            {
              auto base_point = find_base_point();
              if (base_point.first != buffer_end)
              {
                auto first_point = buffer_begin + 1;
                auto second_point = buffer_begin + 2;

                // Found a base point. Now, reorder the buffer such that every three consecutive points
                // represent a path face.
                std::iter_swap(base_point.first, buffer_begin);
                std::iter_swap(base_point.second.front(), first_point);
                std::iter_swap(base_point.second.back(), second_point);

                // Get the location of the next intersection point, and then find a point that's adjacent
                // to one of the ones we already have.
                for (; std::distance(second_point, buffer_end) >= 2; ++first_point, ++second_point)
                {
                  auto range_start = std::next(second_point);
                  auto adjacent_points = find_adjacent_edge_intersections(*first_point, range_start, buffer_end);
                  if (adjacent_points.empty())
                  {
                    adjacent_points = find_adjacent_edge_intersections(*second_point, range_start, buffer_end);
                  }

                  if (!adjacent_points.empty())
                  {
                    std::iter_swap(range_start, adjacent_points.front());
                  }
                }

                for (std::uint32_t idx = 0; idx + 2 < buffer_size; ++idx)
                {
                  output_faces.push_back({
                    buffer_begin[idx].vertex_index,
                    buffer_begin[idx + 1].vertex_index,
                    buffer_begin[idx + 2].vertex_index
                  });
                }
              }
            }
          };

          make_triangle_faces(1, 0, EdgeIntersection::TopRight, 0);
          make_triangle_faces(0, 1, EdgeIntersection::BottomLeft, 1);

          it = range_end;
        }
      }
    }


    template <typename VertexType, typename VertexFunc>
    void generate_path_vertices2(const resources_3d::TrackPath& path,
                                 const resources_3d::StrokeProperties& stroke_properties,
                                 const std::vector<PathVertexPoint>& points,
                                 float texture_size, float texture_z,
                                 const PathCellAlignment& cell_alignment,
                                 resources_3d::BasicModel<VertexType>& output_model,
                                 VertexFunc&& vertex_func)
    {
      using resources_3d::TrackPathNode;
      using resources_3d::StrokeProperties;
      using resources_3d::StrokeSegment;

      std::vector<detail::EdgeIntersection> intersections;

      if (stroke_properties.type == StrokeProperties::Default)
      {
        auto point_it = points.begin();
        if (point_it != points.end())
        {
          auto texture_scale = 1.0f / (stroke_properties.texture_scale * texture_size);
          auto width = resources_3d::path_width_at(*point_it->first, *point_it->second, 
                                                   point_it->time_point) * 0.5f;

          auto make_vertex = [&](Vector2f position)
          {
            PathVertex vertex;
            vertex.position = position;
            vertex.color = stroke_properties.color;
            vertex.normal = { 0.0f, 0.0f, 1.0f };
            vertex.tex_coords.x = position.x * texture_scale;
            vertex.tex_coords.y = position.y * texture_scale;
            vertex.tex_coords.z = texture_z;
            return vertex;
          };

          auto make_transformed_vertex = [&](Vector2f position)
          {
            return vertex_func(make_vertex(position));
          };

          auto intersection_cmp = [](const auto& a, const auto& b)
          {
            return std::tie(a.cell.x, a.cell.y) < std::tie(b.cell.x, b.cell.y);
          };

          auto& output_vertices = output_model.vertices;

          std::array<Vector2f, 4> quad_points =
          {
            point_it->point + point_it->normal * width,
            point_it->point - point_it->normal * width,
            Vector2f(),
            Vector2f()
          };

          const std::array<std::pair<int, int>, 4> edges =
          { {
            { 0, 1 },
            { 0, 2 },
            { 1, 3 },
            { 2, 3 }
          } };

          std::uint32_t edge_index_start = 0;
          std::uint32_t edge_index_end = 4;

          for (auto next_point_it = std::next(point_it); next_point_it != points.end(); ++next_point_it, ++point_it)
          {
            if (distance(point_it->point, next_point_it->point) < 0.001f) continue;

            const auto& next_point = *next_point_it;
            auto next_width = resources_3d::path_width_at(*next_point.first, *next_point.second,
                                                          next_point.time_point) * 0.5f;

            quad_points[2] = next_point.point + next_point.normal * next_width;
            quad_points[3] = next_point.point - next_point.normal * next_width;

            auto vertex_offset = intersections.size();
            for (auto edge_index = edge_index_start; edge_index != edge_index_end; ++edge_index)
            {
              auto edge = edges[edge_index];
              const auto& p1 = quad_points[edge.first];
              const auto& p2 = quad_points[edge.second];

              detail::find_cell_edge_intersections(p1, p2, edge_index, cell_alignment, intersections);
            }

            detail::add_quad_corner_intersections(quad_points, cell_alignment, intersections);
            detail::add_cell_corner_intersections(quad_points, edges, cell_alignment, intersections);

            // Sort the intersections by cell. We need to do this in order to loop through the grid cells
            // in the desired manner.

            detail::add_edge_intersection_vertices(intersections, vertex_offset, output_model, make_transformed_vertex);

            std::sort(intersections.begin(), intersections.end(), intersection_cmp);
            detail::add_edge_intersection_faces(quad_points, edges,
                                                cell_alignment, intersections,
                                                output_model, make_transformed_vertex);

            // Remove all the edge intersections except for the ones with edge index 3,
            // because we're going to need those in the next iteration
            intersections.erase(std::remove_if(intersections.begin(), intersections.end(),
                                               [](const detail::EdgeIntersection& e)
            {
              return e.type == detail::EdgeIntersection::QuadCorner || 
                e.type == detail::EdgeIntersection::CellCorner ||
                e.edge_index != 3;
            }), intersections.end());

            // And transform the edge index to 0, because we the only remaining intersections
            // will have edge index 3, which translates to 0 in the next iteration.
            for (auto& intersection : intersections)
            {
              intersection.edge_index = 0;
            }

            // Make sure we don't use the first edge for all but the first iteration.
            edge_index_start = 1;

            quad_points[0] = quad_points[2];
            quad_points[1] = quad_points[3];
          }
        }
      }
    }

    template <typename VertexType, typename VertexFunc>
    void generate_path_vertices(const resources_3d::TrackPath& path,
                                const resources_3d::StrokeProperties& stroke_properties,
                                const std::vector<PathVertexPoint>& points,
                                float texture_size, float texture_z,
                                resources_3d::BasicModel<VertexType>& output_model,
                                VertexFunc&& vertex_func)
    {
      using resources_3d::TrackPathNode;
      using resources_3d::StrokeProperties;
      using resources_3d::StrokeSegment;

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
                                          output_model, vertex_func);
        }
      }
    }

    template <typename VertexType, typename VertexFunc>
    void generate_path_vertices(const resources_3d::TrackPath& path,
                                const resources_3d::SegmentedStroke& stroke,
                                const std::vector<PathVertexPoint>& points,
                                float texture_size, float texture_z,
                                resources_3d::BasicModel<VertexType>& output_model,
                                VertexFunc&& vertex_func)
    {
      if (stroke.properties.is_segmented)
      {
        detail::generate_path_vertices(path, stroke, points,
                                       texture_size, texture_z,
                                       output_model, vertex_func);
      }

      else
      {
        generate_path_vertices(path, stroke.properties, points,
                               texture_size, texture_z,
                               output_model, vertex_func);
      }
    }
  }
}

#endif