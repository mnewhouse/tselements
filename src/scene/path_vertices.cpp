/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "path_vertices.hpp"

#include "utility/triangle_utilities.hpp"
#include "utility/line_intersection.hpp"
#include "utility/interpolate.hpp"

#include <algorithm>
#include <cstdint>
#include <array>
#include <cmath>
#include <iterator>
#include <fstream>

#include <boost/container/small_vector.hpp>



namespace ts
{
  namespace scene
  {
    namespace detail
    {
      auto make_path_vertex_point_at(PathNodeIterator first_node,
                                     PathNodeIterator second_node,
                                     float time_point)
      {
        PathVertexPoint point;
        point.first = first_node;
        point.second = second_node;
        point.time_point = time_point;
        point.normal = path_normal_at(*first_node, *second_node, time_point);
        point.point = path_point_at(*first_node, *second_node, time_point);
        return point;
      }

      void divide_path_segment(std::vector<PathVertexPoint>& vertex_points,
                               PathNodeIterator first_node, 
                               PathNodeIterator second_node,
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
          auto halfway_point = path_point_at(*first_node, *second_node, time_point);
          auto halfway_normal = path_normal_at(*first_node, *second_node, time_point);

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

      void find_cell_edge_intersections(Vector2f a, Vector2f b,
                                        std::uint32_t edge_index,
                                        const PathCellAlignment& cell_alignment,
                                        std::vector<EdgeIntersection>& edge_intersections)
      {
        auto cell_size = cell_alignment.cell_size;
        auto real_cell_size = static_cast<float>(cell_size);

        // Find the horizontal grid intersections, the vertical ones, and lastly, the diagonal ones.
        auto y_limits = std::minmax(a.y, b.y);
        auto x_limits = std::minmax(a.x, b.x);

        auto diff = b - a;

        // Prepare to loop through all horizontal grid lines between y_limits.first and y_limits.second
        auto y_begin = static_cast<std::int32_t>(std::ceil(y_limits.first / real_cell_size));
        auto y_end = static_cast<std::int32_t>(std::ceil(y_limits.second / real_cell_size));

        for (auto y = y_begin; y < y_end; ++y)
        {
          auto real_y = static_cast<float>(y) * real_cell_size;
          auto intersect_x = a.x + diff.x * (real_y - a.y) / diff.y;

          // Only add the intersection if it's actually on the edge
          if (intersect_x >= x_limits.first && intersect_x <= x_limits.second)
          {
            EdgeIntersection entry;
            entry.cell = { static_cast<std::int32_t>(intersect_x / real_cell_size), y };
            entry.intersection = { intersect_x, real_y };
            entry.type = EdgeIntersection::Horizontal;
            entry.edge_index = edge_index;
            entry.cell_offset = 0;
            edge_intersections.push_back(entry);

            // Now add one more entry for the cell on the other side of the grid line
            --entry.cell.y;
            entry.cell_offset = 1;
            edge_intersections.push_back(entry);
          }
        }

        // Now, do the same for the vertical grid lines.
        auto x_begin = static_cast<std::int32_t>(std::ceil(x_limits.first / real_cell_size));
        auto x_end = static_cast<std::int32_t>(std::ceil(x_limits.second / real_cell_size));

        for (auto x = x_begin; x < x_end; ++x)
        {
          auto real_x = static_cast<float>(x) * real_cell_size;
          auto intersect_y = a.y + diff.y * (real_x - a.x) / diff.x;

          if (intersect_y >= y_limits.first && intersect_y <= y_limits.second)
          {
            EdgeIntersection entry;
            entry.cell = { x, static_cast<std::int32_t>(intersect_y / real_cell_size) };
            entry.intersection = { real_x, intersect_y };
            entry.type = EdgeIntersection::Vertical;
            entry.edge_index = edge_index;
            entry.cell_offset = 0;
            edge_intersections.push_back(entry);

            --entry.cell.x;
            entry.cell_offset = 1;
            edge_intersections.push_back(entry);
          }
        }

        // Now, find the range of diagonal grid lines.
        auto diag_begin = std::min(x_begin, x_end) - std::max(y_begin, y_end);
        auto diag_end = std::max(x_begin, x_end) - std::min(y_begin, y_end);

        auto line_slope = 0.0f;
        auto line_offset = 0.0f;

        const bool vertical = std::abs(diff.x) < 0.00001f;
        if (!vertical)
        {
          line_slope = diff.y / diff.x;
          line_offset = a.y - a.x * line_slope;
        }        

        if (diag_begin > diag_end) std::swap(diag_begin, diag_end);

        for (auto x_offset = diag_begin; x_offset <= diag_end; ++x_offset)
        {
          auto real_offset = x_offset * real_cell_size;
          // Find the location on the line where (x - y) == real_offset.

          boost::optional<Vector2f> intersection;
          if (vertical)
          {
            intersection.emplace(a.x, a.x - real_offset);
          }

          else if (std::abs(line_slope - 1.0f) >= 0.0001f)
          {
            // Equation: intersect_x - (line_slope * intersect_x + line_offset) == real_offset
            auto intersect_x = (-line_offset - real_offset) / (line_slope - 1.0f);
            auto intersect_y = intersect_x * line_slope + line_offset;
            intersection.emplace(intersect_x, intersect_y);
          }

          if (intersection && 
              intersection->x >= x_limits.first && intersection->x <= x_limits.second &&
              intersection->y >= y_limits.first && intersection->y <= y_limits.second)
          {
            EdgeIntersection entry;
            entry.cell.x = static_cast<std::int32_t>(intersection->x) / cell_size;
            entry.cell.y = static_cast<std::int32_t>(intersection->y) / cell_size;
            entry.intersection = { intersection->x, intersection->y };
            entry.type = EdgeIntersection::Diagonal;
            entry.edge_index = edge_index;
            entry.cell_offset = 0;
            edge_intersections.push_back(entry);           
          }
        }
      }

      void add_quad_corner_intersections(const std::array<Vector2f, 4>& points,
                                         const PathCellAlignment& cell_alignment,
                                         std::vector<EdgeIntersection>& edge_intersections)
      {
        std::uint32_t index = 0;
        for (auto point : points)
        {
          EdgeIntersection edge_intersection;
          edge_intersection.cell = vector2_cast<std::int32_t>(point) / cell_alignment.cell_size;

          auto top_left = vector2_cast<float>(edge_intersection.cell * cell_alignment.cell_size);
          auto cell_offset = point - top_left;

          edge_intersection.cell_offset = cell_offset.x >= cell_offset.y ? 1 : 0;
          edge_intersection.intersection = point;
          edge_intersection.type = EdgeIntersection::QuadCorner;
          edge_intersection.edge_index = index;
          edge_intersections.push_back(edge_intersection);

          ++index;
        }
      }

      void add_cell_corner_intersections(const std::array<Vector2f, 4>& points,
                                         const std::array<std::pair<int, int>, 4>& edges,
                                         const PathCellAlignment& cell_alignment,
                                         std::vector<EdgeIntersection>& edge_intersections)
      {
        // Find the cell corners that lie within the quad, and add them to the edge intersection buffer.
        auto edge_sign = [](auto p1, auto p2, auto p3)
        {
          return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y) < 0.0f;
        };

        struct EdgeParameters
        {
          enum Type { Default, Vertical } type;
          float slope;
          float offset;
          bool sign;
        };

        EdgeParameters edge_parameters[4] = {};

        // Find the "sign" of all the edges - that is, which side will the drawable portion be on.
        std::uint32_t edge_index = 0;
        for (const auto& edge : edges)
        {
          const auto& p1 = points[edge.first];
          const auto& p2 = points[edge.second];

          auto diff = p2 - p1;

          auto& params = edge_parameters[edge_index];
          if (std::abs(diff.x) < 0.00001f)
          {
            params.type = EdgeParameters::Vertical;
          }

          else
          {
            params.type = EdgeParameters::Default;
            params.slope = diff.y / diff.x;

            // p1.y = p1.x * slope + offset
            // offset = p1.y - p1.x * slope
            params.offset = p1.y - p1.x * params.slope;
          }

          auto opposing_corner = 0;
          while (opposing_corner == edge.first || opposing_corner == edge.second) ++opposing_corner;

          const auto& opposing_point = points[opposing_corner];

          // Use a dummy outlying point on the bottom side of the edge, and see if that's 
          auto outlying_point = p1; outlying_point.y += 10.0f;
          params.sign = edge_sign(p1, p2, outlying_point) == edge_sign(p1, p2, opposing_point);
          ++edge_index;
        }

        auto cell_size = cell_alignment.cell_size;
        auto real_cell_size = static_cast<float>(cell_size);

        auto x_limits = std::minmax({ points[0].x, points[1].x, points[2].x, points[3].x });
        auto y_limits = std::minmax({ points[0].y, points[1].y, points[2].y, points[3].y });

        auto x_begin = static_cast<std::int32_t>(std::ceil(x_limits.first / real_cell_size));
        auto x_end = static_cast<std::int32_t>(std::ceil(x_limits.second / real_cell_size));

        // Loop through all columns that are within the quad's bounds
        for (auto x = x_begin; x < x_end; ++x)
        {
          // For each edge, find the range of possible y values.
          auto real_x = x * real_cell_size;

          auto min_y = y_limits.first;
          auto max_y = y_limits.second;

          for (const auto& params : edge_parameters)
          {
            if (params.type == EdgeParameters::Default)
            {
              auto real_y = real_x * params.slope + params.offset;
              if (params.sign)
              {
                // Polygon is below this point, so we need to rule out all values above it
                min_y = std::max(min_y, real_y);
              }

              else
              {
                // Polygon is above this point, exclude all values below.
                max_y = std::min(max_y, real_y);
              }
            }
          }

          auto y_begin = static_cast<std::int32_t>(std::ceil(min_y / real_cell_size));
          auto y_end = static_cast<std::int32_t>(std::ceil(max_y / real_cell_size));

          // We have the range of corners, now append everything to the output vector
          for (auto y = y_begin; y < y_end; ++y)
          {
            auto real_y = y * real_cell_size;

            EdgeIntersection edge_intersection;
            edge_intersection.cell = { x - 1, y - 1 };
            edge_intersection.intersection = { real_x, real_y };
            edge_intersection.edge_index = EdgeIntersection::BottomRight;
            edge_intersection.cell_offset = 0;
            edge_intersection.type = EdgeIntersection::CellCorner;

            edge_intersections.push_back(edge_intersection);

            ++edge_intersection.cell.y;
            edge_intersection.edge_index = EdgeIntersection::TopRight;
            edge_intersections.push_back(edge_intersection);

            --edge_intersection.cell.y;
            ++edge_intersection.cell.x;
            edge_intersection.edge_index = EdgeIntersection::BottomLeft;
            edge_intersections.push_back(edge_intersection);

            ++edge_intersection.cell.y;
            edge_intersection.edge_index = EdgeIntersection::TopLeft;
            edge_intersections.push_back(edge_intersection);            
          }
        }
      }
    }

    // Precompute the points at which path vertices will be generated.
    // This is useful when a path has more than one stroke type, 
    // Lower tolerance values will give smoother results at the cost of more vertices.
    // 0.0 < tolerance <= 1.0, but it should not be too close to zero.
    void compute_path_vertex_points(PathNodeIterator node_it,
                                    PathNodeIterator node_end,
                                    float tolerance,
                                    std::vector<PathVertexPoint>& vertex_points)
    {
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
            vertex_points.push_back(*point_it);

            // Recursively divide the segment until it's smooth enough.
            detail::divide_path_segment(vertex_points, node_it, next_node_it,
                                        point_it->time_point, next_point_it->time_point,
                                        point_it->point, next_point_it->point,
                                        point_it->normal, next_point_it->normal,
                                        tolerance);
          }

          vertex_points.push_back(*point_it);
        }
      }
    }
  }
}