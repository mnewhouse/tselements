/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "model_3d.hpp"

#include "utility/vector2.hpp"
#include "utility/color.hpp"
#include "utility/rect.hpp"
#include "utility/triangle_utilities.hpp"

#include "graphics/texture.hpp"

#include <boost/geometry/index/rtree.hpp>

#include <utility>
#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>

namespace ts
{
  namespace resources3d
  {
    class ElevationMap;

    struct TerrainModel
    {
      struct Component
      {
        std::uint32_t face_index;
        std::uint32_t face_count;
        std::size_t model_tag;
      };

      std::vector<Vertex> vertices;
      std::vector<Face> faces;
      std::vector<Component> components;
    };

    namespace detail
    {
      struct ModelInfo
      {
        std::size_t model_tag;
      };

      struct ModelVertex
      {
        Vector2<std::int64_t> integral_point;
        Vertex vertex;
      };

      struct ModelFace
      {
        const ModelVertex& a;
        const ModelVertex& b;
        const ModelVertex& c;
      };

      struct EdgeIntersection
      {
        Vector2<std::int64_t> point;
        std::array<std::uint32_t, 4> edges;
        std::uint32_t vertex_index;
      };
    }
    
    class TerrainBuilder
    {
    public:
      void apply_model(const Model& model, std::size_t model_tag);

      TerrainModel build_model(const ElevationMap& elevation_map) const;

    private:
      void apply_face(const std::array<std::uint32_t, 3>& face_indices, std::uint32_t model_id);

      using point = boost::geometry::model::point<std::int64_t, 2, boost::geometry::cs::cartesian>;
      using rtree_box = boost::geometry::model::box<point>;
      using rtree_parameters = boost::geometry::index::quadratic<32>;
      using rtree_value = std::pair<rtree_box, std::array<std::uint32_t, 4>>;
      using geometry_rtree = boost::geometry::index::rtree<rtree_value, rtree_parameters>;
      
      static rtree_box make_triangle_box(const Triangle2<std::int64_t>& triangle);

      geometry_rtree geometry_;

      std::vector<detail::ModelInfo> model_info_;
      std::vector<detail::ModelVertex> vertices_;

      std::vector<rtree_value> query_buffer_;
      std::vector<rtree_value> insertion_buffer_;

      std::vector<std::pair<std::uint32_t, std::uint32_t>> edge_buffer_;
      std::vector<detail::EdgeIntersection> intersection_buffer_;
    };
  }
}