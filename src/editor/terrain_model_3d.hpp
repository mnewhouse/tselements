/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TERRAIN_3D_HPP_849184718
#define TERRAIN_3D_HPP_849184718

#include "model_3d.hpp"

#include "utility/rect.hpp"

#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>

namespace ts
{
  namespace resources_3d
  {
    class TerrainModel
    {
    public:
      TerrainModel() = default;
      explicit TerrainModel(Model model);

      template <typename Func>
      void for_each_face_in_area(FloatRect area, Func&& func) const;

      template <typename Func>
      void for_each_face_at_point(Vector2f point, Func&& func) const;

      boost::optional<FaceVertices> find_face_at_point(Vector2f point) const;

      boost::optional<float> height_at_point(Vector2f point) const;

      const Model& model() const;

    private:
      void regenerate_lookup_map();

      Model model_;     

      using index_point_type = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>;
      using index_box_type = boost::geometry::model::box<index_point_type>;
      using index_value = std::pair<index_box_type, ModelFace>;
      boost::geometry::index::rtree<index_value, boost::geometry::index::quadratic<16>> face_lookup_map_;
    };

    boost::optional<float> height_at_point(const FaceVertices& vertices, Vector2f point);
  }
}

#endif