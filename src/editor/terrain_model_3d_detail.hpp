/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TERRAIN_MODEL_3D_DETAIL_HPP_237821374581
#define TERRAIN_MODEL_3D_DETAIL_HPP_237821374581

#include "terrain_model_3d.hpp"

#include <boost/function_output_iterator.hpp>

namespace ts
{
  namespace resources_3d
  {
    template <typename Func>
    void TerrainModel::for_each_face_in_area(FloatRect area, Func&& func) const
    {
      index_box_type box(index_point_type(area.left, area.top),
                         index_point_type(area.right(), area.bottom()));

      auto out = boost::make_function_output_iterator([&](const index_value& pair)
      {
        func(face_vertices(model_, pair.second));
      });

      face_lookup_map_.query(boost::geometry::index::intersects(box), out);
    }
  }
}

#endif