/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "path_vertices_3d.hpp"
#include "path_utilities_3d.hpp"
#include "elevation_map_3d.hpp"

#include <boost/range/adaptor/reversed.hpp>
#include <boost/optional.hpp>

#include <algorithm>

namespace ts
{
  namespace resources3d
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

      void divide_outline_segment(std::vector<PathVertexPoint>& vertex_points,
                                  PathNodeIterator first_node,
                                  PathNodeIterator second_node,
                                  float min_time_point, float max_time_point,
                                  Vector2f first_point, Vector2f second_point,
                                  Vector2f first_normal, Vector2f second_normal,
                                  float tolerance, bool negate_normal)
      {
        if ((std::abs(first_point.x - second_point.x) >= 1.0f ||
            std::abs(first_point.y - second_point.y) >= 1.0f) &&
            std::abs(max_time_point - min_time_point) >= 0.001f)
        {
          auto time_interval = max_time_point - min_time_point;
          auto time_point = min_time_point + time_interval * 0.5f;

          auto halfway_width = path_width_at(*first_node, *second_node, time_point);
          auto halfway_normal = path_normal_at(*first_node, *second_node, time_point);

          PathVertexPoint halfway_vp;
          halfway_vp.first = first_node;
          halfway_vp.second = second_node;
          halfway_vp.normal = halfway_normal;
          if (negate_normal) halfway_vp.normal = -halfway_vp.normal;

          halfway_vp.point = path_point_at(*first_node, *second_node, time_point) + halfway_vp.normal * halfway_width;

          // Use the square of the dot product of the normals to
          // determine how well they match. If 1.0 - dp² is larger than the tolerance,
          // proceed with the recursion.
          auto first_dp = std::abs(dot_product(first_normal, halfway_normal));
          if (1.0 - first_dp * first_dp > tolerance)
          {
            divide_outline_segment(vertex_points, first_node, second_node,
                                   min_time_point, time_point,
                                   first_point, halfway_vp.point,
                                   first_normal, halfway_normal,
                                   tolerance, negate_normal);
          }

          vertex_points.push_back(halfway_vp);

          auto second_dp = std::abs(dot_product(halfway_normal, second_normal));
          if (1.0 - second_dp * second_dp > tolerance)
          {
            divide_outline_segment(vertex_points, first_node, second_node,
                                   time_point, max_time_point,
                                   halfway_vp.point, second_point,
                                   halfway_normal, second_normal,
                                   tolerance, negate_normal);
          }
        }
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
    }

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

    void generate_path_outline(PathNodeIterator node_it, PathNodeIterator node_end, float tolerance, PathOutline& outline)
    {
      outline.first.clear();
      outline.second.clear();

      if (node_it != node_end)
      {
        auto next_node_it = std::next(node_it);
        for (; next_node_it != node_end; ++node_it, ++next_node_it)
        {
          PathVertexPoint points[] =
          {
            detail::make_path_vertex_point_at(node_it, next_node_it, 0.0f),
            detail::make_path_vertex_point_at(node_it, next_node_it, 0.5f),
            detail::make_path_vertex_point_at(node_it, next_node_it, 1.0f)
          };

          auto offset_point = [](PathVertexPoint vp, float offset)
          {
            vp.point += vp.normal * offset;
            return vp;
          };

          auto point_it = std::begin(points);
          auto next_point_it = std::next(point_it);

          auto next_width = path_width_at(*point_it->first, *point_it->second, point_it->time_point);
          auto next_left = offset_point(*point_it, -next_width);
          auto next_right = offset_point(*point_it, next_width);

          int i = 0;
          for (; next_point_it != std::end(points); ++point_it, ++next_point_it)
          {
            auto left = next_left;
            auto right = next_right;

            outline.first.push_back(left);
            outline.second.push_back(right);

            next_width = path_width_at(*next_point_it->first, *next_point_it->second, next_point_it->time_point);
            next_left = offset_point(*next_point_it, -next_width);
            next_right = offset_point(*next_point_it, next_width);

            detail::divide_outline_segment(outline.first, point_it->first, point_it->second,
                                           point_it->time_point, next_point_it->time_point,
                                           left.point, next_left.point,
                                           left.normal, next_left.normal, tolerance, true);

            detail::divide_outline_segment(outline.second, point_it->first, point_it->second,
                                           point_it->time_point, next_point_it->time_point,
                                           right.point, next_right.point,
                                           right.normal, next_right.normal, tolerance, false);
          }
        }
      }
    }

    Model build_path_model(const PathStrokeStyle& stroke_style, const std::vector<PathVertexPoint>& vertex_points,
                           const ElevationMap& elevation_map, float texture_z, Vector2f texture_scale, bool closed)
    {
      Model model;

      if (vertex_points.size() >= 2)
      {
        auto color = stroke_style.color;

        if (stroke_style.type == PathStrokeStyle::Border)
        {

        }

        else if (stroke_style.type == PathStrokeStyle::Regular)
        {
          for (const auto& vp : vertex_points)
          {
            auto make_vertex = [=](Vector2f point)
            {
              Vertex v;
              v.position = make_3d(point, 0.0f);
              v.normal = { 0.0f, 0.0f, 1.0f };
              v.color = color;
              v.tex_coords = make_3d(point * texture_scale, texture_z);
              return v;
            };

            auto width = path_width_at(*vp.first, *vp.second, vp.time_point) * 0.5f;

            model.vertices.push_back(make_vertex(vp.point + vp.normal * width));
            model.vertices.push_back(make_vertex(vp.point - vp.normal * width));
          }


          auto point_it = vertex_points.begin();
          auto next_point_it = std::next(point_it);

          std::uint32_t vertex_index = 0;
          for (; next_point_it != vertex_points.end(); ++point_it, ++next_point_it, vertex_index += 2)
          {
            model.faces.push_back({ vertex_index, vertex_index + 1, vertex_index + 2 });
            model.faces.push_back({ vertex_index + 1, vertex_index + 2, vertex_index + 3 });
          }
        }
      }

      return model;
    }

    /*
    Outline generate_path_outline(const PathStrokeStyle& stroke_style, 
                                  const std::vector<PathVertexPoint>& vertex_points,
                                  const ElevationMap& elevation_map,
                                  bool closed)
    {
      Outline outline;

      if (vertex_points.size() >= 2 && stroke_style.type == PathStrokeStyle::Regular)
      {
        // Generate two outlines, one for each side of the track.
        outline.first.reserve(vertex_points.size());
        outline.second.reserve(vertex_points.size());

        auto make_point = [](const PathVertexPoint& vp, float normal_multiplier)
        {
          auto width = path_width_at(*vp.first, *vp.second, vp.time_point);

          OutlinePoint p;
          p.point = vp.point + vp.normal * width * normal_multiplier * 0.5f;
          p.normal = vp.normal * normal_multiplier;
          return p;
        };

        for (auto& vp : vertex_points)
        {
          outline.first.push_back(make_point(vp, 1.0f));
          outline.second.push_back(make_point(vp, -1.0f));
        }
      }

      return outline;
    }

    void find_contained_cell_corners(const Outline& outline, const CellAlignment& cell_alignment, 
                                     std::vector<CellCorner>& cell_corners)
    {
      auto length = outline.first.size();
      if (length == outline.second.size())
      {        
        auto cell_size = static_cast<float>(cell_alignment.cell_size);

        // Loop through both sides of the outline, testing every quad for their cell corners.
        for (std::size_t index = 0, next_index = 1; next_index < length; ++index, ++next_index)
        {
          const auto a = outline.first[index].point;
          const auto b = outline.first[next_index].point;
          const auto c = outline.second[next_index].point;
          const auto d = outline.second[index].point;

          auto minmax_y = std::minmax({ a.y, b.y, c.y, d.y });
          auto minmax_x = std::minmax({ a.x, b.x, c.x, d.x });

          auto min_cell_y = static_cast<std::int32_t>(std::floor(minmax_y.first / cell_size));
          auto max_cell_y = static_cast<std::int32_t>(std::ceil(minmax_y.second / cell_size));

          auto min_cell_x = static_cast<std::int32_t>(std::floor(minmax_x.first / cell_size));
          auto max_cell_x = static_cast<std::int32_t>(std::ceil(minmax_x.second / cell_size));

          const Vector2f ev[4] = { b - a, c - b, d - c, a - d };

          // Simply loop through the rectangular bounding box and check if the point lies within the quad.
          // I'm sure there are more efficient ways to do this, but this is easy and probably fast enough.
          for (auto cell_y = min_cell_y; cell_y < max_cell_y; ++cell_y)
          {
            auto real_y = cell_y * cell_size;
            auto center_y = (cell_y + 0.5f) * cell_size;

            for (auto cell_x = min_cell_x; cell_x < max_cell_x; ++cell_x)
            {
              auto real_x = cell_x * cell_size;
              auto center_x = (cell_x + 0.5f) * cell_size;             

              auto test = [&](Vector2f p)
              {
                const bool signs[4] =
                {
                  std::signbit(cross_product(ev[0], p - a)),
                  std::signbit(cross_product(ev[1], p - b)),
                  std::signbit(cross_product(ev[2], p - c)),
                  std::signbit(cross_product(ev[3], p - d))
                };

                return signs[0] == signs[1] && signs[0] == signs[2] && signs[0] == signs[3];
              };

              CellCorner cell_corner = {};
              cell_corner.cell = { cell_x, cell_y };

              if (test(make_vector2(real_x, real_y)))
              {
                cell_corner.point = { real_x, real_y };
                cell_corner.type = CellCorner::Corner;
                cell_corners.push_back(cell_corner);
              }

              if (test(make_vector2(center_x, center_y)))
              {
                cell_corner.point = { center_x, center_y };
                cell_corner.type = CellCorner::Center;
                cell_corners.push_back(cell_corner);
              }
            }
          }
        }

        auto cmp_less = [](const CellCorner& a, const CellCorner& b)
        {
          return std::tie(a.cell.y, a.cell.x, a.type) < std::tie(b.cell.y, b.cell.x, b.type);
        };

        auto cmp_equal = [](const CellCorner& a, const CellCorner& b)
        {
          return std::tie(a.cell.y, a.cell.x, a.type) == std::tie(b.cell.y, b.cell.x, b.type);
        };

        // Now, sort the stuff and remove the duplicate entries.
        std::sort(cell_corners.begin(), cell_corners.end(), cmp_less);
        cell_corners.erase(std::unique(cell_corners.begin(), cell_corners.end(), cmp_equal), cell_corners.end());
      }
    }

    void find_cell_edge_intersections(Vector2f a, Vector2f b, std::uint32_t edge_index,
                                      const CellAlignment& cell_alignment,
                                      std::vector<EdgeIntersection>& intersections)
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
          entry.cell = { static_cast<std::int32_t>(std::floor(intersect_x / real_cell_size)), y };
          entry.intersect_point = { intersect_x, real_y };
          entry.type = EdgeIntersection::Horizontal;
          entry.cell_offset = 0;
          entry.edge_index = edge_index;
          intersections.push_back(entry);

          // Now add one more entry for the cell on the other side of the grid line
          --entry.cell.y;
          entry.cell_offset = 1;
          intersections.push_back(entry);
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
          entry.cell = { x, static_cast<std::int32_t>(std::floor(intersect_y / real_cell_size)) };
          entry.intersect_point = { real_x, intersect_y };
          entry.type = EdgeIntersection::Vertical;
          entry.cell_offset = 0;
          entry.edge_index = edge_index;
          intersections.push_back(entry);

          --entry.cell.x;
          entry.cell_offset = 1;
          intersections.push_back(entry);
        }
      }

      auto line_slope = 0.0f;
      auto line_offset = 0.0f;

      const bool vertical = std::abs(diff.x) < 0.00001f;
      if (!vertical)
      {
        line_slope = diff.y / diff.x;
        line_offset = a.y - a.x * line_slope;
      }

      // Now, find the range of diagonal grid lines.
      auto tlbr_begin = std::min(x_begin, x_end) - std::max(y_begin, y_end);
      auto tlbr_end = std::max(x_begin, x_end) - std::min(y_begin, y_end);

      auto bltr_begin = -tlbr_begin;
      auto bltr_end = -tlbr_end;

      if (tlbr_begin > tlbr_end) std::swap(tlbr_begin, tlbr_end);
      if (bltr_begin > bltr_end) std::swap(bltr_begin, bltr_end);

      auto diagonal_loop = [&](auto begin, auto end, auto offset_multiplier, auto intersection_type)
      {
        for (auto x_offset = begin; x_offset <= end; ++x_offset)
        {
          auto real_offset = x_offset * offset_multiplier;

          boost::optional<Vector2f> intersection;
          if (vertical) intersection.emplace(a.x, a.x - real_offset);

          else
          {
            auto intersect_x = (line_offset + real_offset) / (1.0f - line_slope);
            auto intersect_y = intersect_x * line_slope + line_offset;
            intersection.emplace(intersect_x, intersect_y);
          }

          if (intersection &&
              intersection->x >= x_limits.first && intersection->x <= x_limits.second &&
              intersection->y >= y_limits.first && intersection->y <= y_limits.second)
          {
            EdgeIntersection entry;
            entry.cell.x = static_cast<std::int32_t>(std::floor(intersection->x / cell_size));
            entry.cell.y = static_cast<std::int32_t>(std::floor(intersection->y / cell_size));
            entry.intersect_point = { intersection->x, intersection->y };
            entry.type = intersection_type;
            entry.edge_index = edge_index;
            entry.cell_offset = 0;
            intersections.push_back(entry);
          }
        }
      };

      diagonal_loop(tlbr_begin, tlbr_end, real_cell_size, EdgeIntersection::Diagonal_TL_BR);
      diagonal_loop(bltr_begin, bltr_end, -real_cell_size, EdgeIntersection::Diagonal_BL_TR);
    }

    void find_cell_edge_intersections(const Outline& outline, const CellAlignment& cell_alignment,
                                      std::vector<EdgeIntersection>& intersections)
    {
      auto length = outline.first.size();
      if (length >= 2 && length == outline.second.size())
      {
        const auto& first = outline.first;
        const auto& second = outline.second;


        find_cell_edge_intersections(first[0].point, second[0].point, 0, cell_alignment, intersections);
        std::uint32_t edge_index = 1;

        for (std::size_t index = 0, next_index = 1; next_index < length; ++index, ++next_index, edge_index += 2)
        {
          find_cell_edge_intersections(first[index].point, first[next_index].point, edge_index, cell_alignment, intersections);
          find_cell_edge_intersections(second[index].point, second[next_index].point, edge_index + 1, cell_alignment, intersections);
        }

        find_cell_edge_intersections(first.back().point, second.back().point, edge_index, cell_alignment, intersections);
      }
    }

    std::vector<EdgeIntersection> find_cell_edge_intersections(const Outline& outline, const CellAlignment& cell_alignment)

    {
      std::vector<EdgeIntersection> intersections;
      find_cell_edge_intersections(outline, cell_alignment, intersections);

      return intersections;
    }

    void sort_cell_edge_intersections(std::vector<EdgeIntersection>& intersections)
    {
      std::sort(intersections.begin(), intersections.end(), 
                [](const EdgeIntersection& a, const EdgeIntersection& b)
      {
        return std::tie(a.cell.y, a.cell.x) < std::tie(b.cell.y, b.cell.x);
      });
    }

    void build_path_model(const std::vector<EdgeIntersection>& edge_intersections, const CellAlignment& cell_alignment,
                          const ElevationMap& elevation_map, Model& model)
    {
      const auto begin_it = edge_intersections.begin();
      const auto end_it = edge_intersections.end();

      // We expect the input vector to be sorted properly. Now, we loop through each distinct cell,
      // and add the triangle faces we need. Then, for every "in-between" cell, simply add
      // the four triangles that the terrain grid is logically made up of.
      for (auto it = begin_it; it != end_it; )
      {
        auto cell = it->cell;

        // Find the end of the cell range, that is, the first entry after 'it' 
        // whose cell is not the same as it->cell.
        auto cell_end = std::find_if(std::next(it), end_it, 
                                     [=](const EdgeIntersection& e)
        {
          return e.cell != cell;
        });

        
        it = cell_end;
      }
    }
    */
  }
}