/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "track_path_3d.hpp"
#include "model_3d.hpp"

#include <cstdint>

namespace ts
{
  namespace resources3d
  {
    class ElevationMap;

    using PathNodeIterator = std::vector<PathNode>::const_iterator;

    struct PathVertexPoint
    {
      PathNodeIterator first;
      PathNodeIterator second;
      float time_point = 0.0f;
      Vector2f point;
      Vector2f normal;
    };

    struct PathOutline
    {
      std::vector<PathVertexPoint> first;
      std::vector<PathVertexPoint> second;
    };

    struct EdgeIntersection
    {
      enum Type
      {
        Horizontal,
        Vertical,
        Diagonal_TL_BR,
        Diagonal_BL_TR,
        Vertex
      };

      Type type;
      Vector2i cell;
      Vector2f intersect_point;
      std::uint32_t edge_index = 0;
      std::uint32_t vertex_index = 0;
      std::int32_t cell_offset = 0;
    };

    struct CellCorner
    {
      enum Type { Corner, Center };

      Type type;
      Vector2i cell;
      Vector2f point;
      std::uint32_t vertex_index;
    };

    struct CellAlignment
    {
      bool align = true;
      std::int32_t cell_size;
    };

    // Precompute the points at which path vertices will be generated.
    // This is useful when a path has more than one stroke type, 
    // Lower tolerance values will give smoother results at the cost of more vertices.
    // 0.0 < tolerance <= 1.0, but it should not be too close to zero. (i.e. not below +- 0.01)
    void compute_path_vertex_points(PathNodeIterator node_it,
                                    PathNodeIterator node_end,
                                    float tolerance,
                                    std::vector<PathVertexPoint>& vertex_points);

    void generate_path_outline(PathNodeIterator node_it, PathNodeIterator node_end, float tolerance,
                               PathOutline& outline);

    Model build_path_model(const PathStrokeStyle& stroke_style, const std::vector<PathVertexPoint>& vertex_points,
                           const ElevationMap& elevation_map, float texture_z, Vector2f texture_scale, bool closed);

    /*

    void find_contained_cell_corners(const Outline& outline, const CellAlignment& cell_alignment,
                                     std::vector<CellCorner>& cell_corners);

    void find_contained_cell_corners(const Outline& outline, const CellAlignment& cell_alignment,
                                     std::vector<CellCorner>& cell_corners);

    void find_cell_edge_intersections(const Outline& outline, const CellAlignment& cell_alignment,
                                      std::vector<EdgeIntersection>& edge_intersections);

    std::vector<EdgeIntersection> find_cell_edge_intersections(const Outline& outline, 
                                                               const CellAlignment& cell_alignment);

    // Sort the cell edge intersections by their corresponding cell. The comparison function is
    // equivalent to std::tie(a.cell.y, a.cell.x) < std::tie(b.cell.y, b.cell.x).
    void sort_cell_edge_intersections(std::vector<EdgeIntersection>& intersections);

    // Build a path model according to the edge intersections we found. The input vector is expected to have
    // been sorted as if by calling sort_cell_edge_intersections().
    Model build_path_model(const std::vector<EdgeIntersection>& edge_intersections, const CellAlignment& cell_alignment,
                           const ElevationMap& elevation_map);
                           */
  }
}