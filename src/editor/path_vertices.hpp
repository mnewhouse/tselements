/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef PATH_VERTICES_HPP_384912834
#define PATH_VERTICES_HPP_384912834

#include "track_path.hpp"

#include <vector>
#include <cstdint>

namespace ts
{
  namespace scene_3d
  {
    template <typename NodeIt>
    struct PathVertexPoint
    {
      NodeIt first;
      NodeIt second;
      float time_point = 0.0f;
      Vector2f point;
      Vector2f normal;
    };

    // Precompute the points at which path vertices will be generated.
    // This is useful when a path has more than one stroke type, 
    // Lower tolerance values will give smoother results at the cost of more vertices.
    // 0.0 < tolerance <= 1.0, but it should not be too close to zero.
    // PathNodeIt must be a forward iterator that dereferences to resources_3d::TrackPathNode.
    template <typename PathNodeIt>
    auto compute_path_vertex_points(PathNodeIt node_it, PathNodeIt end, float tolerance,
                                    std::vector<PathVertexPoint<PathNodeIt>>& result);

    // Generate the vertices according to the vertex points previously generated by
    // compute_path_vertex_points. It writes the vertices and indices to the given
    // output iterators. 
    // The following syntax must be supported: *vertex_out++ = vertex_func(Vector2f(position))
    // It also performs the operation *index_out++ = std::size_t(start_index + index)
    template <typename PathNodeIt, typename VertexFunc, typename VertexOut, typename IndexOut>
    auto generate_path_vertices(const std::vector<PathVertexPoint<PathNodeIt>>& points,
                                PathNodeIt first_node,
                                const resources_3d::SegmentedStroke& stroke,
                                VertexFunc&& vertex_func, VertexOut vertex_out,
                                std::uint32_t start_index, IndexOut index_out);

    template <typename PathNodeIt, typename VertexFunc, typename VertexOut, typename IndexOut>
    auto generate_path_vertices(const std::vector<PathVertexPoint<PathNodeIt>>& points,
                                const resources_3d::StrokeProperties& stroke_properties,
                                VertexFunc&& vertex_func, VertexOut vertex_out,
                                std::uint32_t start_index, IndexOut index_out);
  }
}

#endif