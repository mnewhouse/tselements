/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resources/track_path.hpp"
#include "resources/geometry.hpp"

#include <limits>
#include <vector>
#include <cstdint>

/*
namespace ts
{
  namespace scene
  {
    using PathNodeIterator = std::vector<resources::TrackPathNode>::const_iterator;

    struct PathVertex
    {
      Vector2f position;
      Vector3f tex_coords;
      Vector3f normal;
      Colorb color;
    };

    // Precompute the points at which path vertices will be generated.
    // This is useful when a path has more than one stroke type, 
    // Lower tolerance values will give smoother results at the cost of more vertices.
    // 0.0 < tolerance <= 1.0, but it should not be too close to zero. (i.e. not below approx. 0.01)
    void compute_path_vertex_points(PathNodeIterator node_it,
                                    PathNodeIterator node_end,
                                    float tolerance,
                                    std::vector<PathVertexPoint>& vertex_points);

    // Generate the model according to the vertex points previously generated by
    // compute_path_vertex_points.
    // * Requirements
    // * VertexFunc must be a function that takes a single PathVertex parameter and
    //   returns something that's convertible to VertexType.
    template <typename VertexType, typename VertexTransform>
    void generate_path_vertices(const resources::TrackPath& path,
                                const resources::SegmentedStrokeStyle& stroke,
                                const std::vector<PathVertexPoint>& points,
                                float texture_size, float texture_z,
                                resources::BasicGeometry<VertexType>& output_model,
                                VertexTransform&& transform_vertex);

    template <typename VertexType, typename VertexTransform>
    void generate_path_vertices(const resources::TrackPath& path,
                                const resources::StrokeStyle& stroke_properties,
                                const std::vector<PathVertexPoint>& points,
                                float texture_size, float texture_z,
                                resources::BasicGeometry<VertexType>& output_geometry,
                                VertexTransform&& transform_vertex);
  }
}
*/