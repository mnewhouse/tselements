/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "terrain_model_3d.hpp"

#include "utility/triangle_utilities.hpp"

namespace ts
{
  namespace resources_3d
  {
    TerrainModel::TerrainModel(Model model)
      : model_(std::move(model))
    {
      regenerate_lookup_map();
    }

    const resources_3d::Model& TerrainModel::model() const
    {
      return model_;
    }

    void TerrainModel::regenerate_lookup_map()
    {
      face_lookup_map_.clear();
      for (const auto& face : model_.faces)
      {
        auto vertices = face_vertices(model_, face);

        auto points =
        {
          make_2d(vertices.first.position),
          make_2d(vertices.second.position),
          make_2d(vertices.third.position)
        };

        auto bounding_box = make_rect_from_points(points);
        auto box_entry = index_box_type(index_point_type(bounding_box.left, bounding_box.top),
                                        index_point_type(bounding_box.right(), bounding_box.bottom()));

        face_lookup_map_.insert(std::make_pair(box_entry, face));
      }
    }


    boost::optional<float> TerrainModel::height_at_point(Vector2f point) const
    {

      index_point_type p(point.x, point.y);

      auto sign = [](auto p1, auto p2, auto p3)
      {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
      };

      boost::optional<float> result;

      auto query_it = face_lookup_map_.qbegin(boost::geometry::index::contains(p));
      for (; query_it != face_lookup_map_.qend(); ++query_it)
      {
        auto face_verts = face_vertices(model_, query_it->second);
        auto triangle = make_triangle(make_2d(face_verts.first.position),
                                      make_2d(face_verts.second.position),
                                      make_2d(face_verts.third.position));

        if (auto height = resources_3d::height_at_point(face_verts, point))
        {
          if (!result) result = height;
          else if (triangle_contains(triangle, point) && *height > *result) *result = *height;
        }
      }

      return result;
    }

    boost::optional<float> height_at_point(const FaceVertices& vertices, Vector2f point)
    {
      auto p = vector3_cast<double>(make_3d(point));

      auto p1 = p - vector3_cast<double>(vertices.first.position);
      auto p2 = p - vector3_cast<double>(vertices.second.position);
      auto p3 = p - vector3_cast<double>(vertices.third.position);

      // Thanks Wolfram|Alpha
      auto a = p1.x, b = p1.y, c = -p1.z, d = p2.x, e = p2.y, f = -p2.z, g = p3.x, h = p3.y, i = -p3.z;

      auto det = -a * e + a * h + b * d - b * g - d * h + e * g;
      if (det == 0.0) return boost::none;

      auto r = (-a * e * i + a * f * h + b * d * i - b * f * g - c * d * h + c * e * g) / det;
      return static_cast<float>(r);
    }
  }
}