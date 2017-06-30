/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "terrain_model_3d.hpp"
#include "elevation_map_3d.hpp"

#include "utility/debug_log.hpp"

#include <boost/container/small_vector.hpp>

#include <cstdint>
#include <numeric>
#include <iostream>

namespace ts
{
  namespace resources3d
  {
    namespace detail
    {
      auto make_integral(const Vector2f& point)
      {
        return vector2_round<std::int64_t>(point * 256.0);
      }

      auto make_integral(const Vector3f& point)
      {
        return make_integral(make_2d(point));
      }

      std::array<float, 3> barycentric_weight(const Vector2<std::int64_t>& point,
                                              const Vector2<std::int64_t>& a, const Vector2<std::int64_t>& b, const Vector2<std::int64_t>& c)
      {
        auto total_area = std::abs(cross_product(b - a, c - a));
        auto inverse_area = 1.0f / total_area;

        auto a_total = std::abs(cross_product(c - b, point - b));
        auto b_total = std::abs(cross_product(a - c, point - c));
        auto c_total = std::abs(cross_product(b - a, point - a));

        return{ { a_total * inverse_area, b_total * inverse_area, c_total * inverse_area } };
      }

      ModelVertex interpolate_vertex(const Vector2<std::int64_t>& point,
                                     const ModelVertex& a, const ModelVertex& b, const ModelVertex& c)
      {
        auto weight = barycentric_weight(point, a.integral_point, b.integral_point, c.integral_point);

        ModelVertex result;
        result.integral_point = point;
        result.vertex.position = make_3d(vector2_cast<float>(point / 256.0));
        result.vertex.normal = a.vertex.normal * weight[0] + b.vertex.normal * weight[1] + c.vertex.normal * weight[2];
        result.vertex.tex_coords = a.vertex.tex_coords * weight[0] + b.vertex.tex_coords * weight[1] + c.vertex.tex_coords * weight[3];
        result.vertex.color =
        {
          static_cast<std::uint8_t>(a.vertex.color.r * weight[0] + b.vertex.color.r * weight[1] + c.vertex.color.r * weight[2]),
          static_cast<std::uint8_t>(a.vertex.color.g * weight[0] + b.vertex.color.g * weight[1] + c.vertex.color.g * weight[2]),
          static_cast<std::uint8_t>(a.vertex.color.b * weight[0] + b.vertex.color.b * weight[1] + c.vertex.color.b * weight[2]),
          static_cast<std::uint8_t>(a.vertex.color.a * weight[0] + b.vertex.color.a * weight[1] + c.vertex.color.a * weight[2])
        };

        return result;
      }

      ModelVertex interpolate_vertex(const Vector2<std::int64_t>& point, const ModelVertex& a, const ModelVertex& b)
      {
        auto line_diff = b.integral_point - a.integral_point;
        auto length = line_diff.x * line_diff.x + line_diff.y * line_diff.y;

        auto b_weight = static_cast<float>(std::abs(dot_product(point - a.integral_point, line_diff)) / static_cast<double>(length));
        auto a_weight = 1.0f - b_weight;

        ModelVertex result;
        result.integral_point = point;
        result.vertex.position = make_3d(vector2_cast<float>(point / 256.0));
        result.vertex.normal = a.vertex.normal * a_weight + b.vertex.normal * b_weight;
        result.vertex.tex_coords = a.vertex.tex_coords * a_weight + b.vertex.tex_coords * b_weight;
        result.vertex.color =
        {
          static_cast<std::uint8_t>(a.vertex.color.r * a_weight + b.vertex.color.r * b_weight),
          static_cast<std::uint8_t>(a.vertex.color.g * a_weight + b.vertex.color.g * b_weight),
          static_cast<std::uint8_t>(a.vertex.color.b * a_weight + b.vertex.color.b * b_weight),
          static_cast<std::uint8_t>(a.vertex.color.a * a_weight + b.vertex.color.a * b_weight)
        };
        
        return result;
      }
    }

    void TerrainBuilder::apply_model(const Model& model, std::size_t model_tag)
    {
      auto model_id = static_cast<std::uint32_t>(model_info_.size());

      detail::ModelInfo model_info;
      model_info.model_tag = model_tag;
      model_info_.push_back(model_info);

      auto vertex_index = static_cast<std::uint32_t>(vertices_.size());
      vertices_.resize(vertices_.size() + model.vertices.size());

      std::transform(model.vertices.begin(), model.vertices.end(), vertices_.begin() + vertex_index, 
                     [](const Vertex& v)
      {
        detail::ModelVertex r;
        r.vertex = v;
        r.integral_point = detail::make_integral(v.position);
        return r;
      });

      for (const auto& face : model.faces)
      {
        apply_face({ face.a + vertex_index, face.b + vertex_index, face.c + vertex_index }, model_id);
      }
    }
    
    TerrainBuilder::rtree_box TerrainBuilder::make_triangle_box(const Triangle2<std::int64_t>& triangle)
    {
      auto minmax_x = std::minmax({ triangle[0].x, triangle[1].x, triangle[2].x });
      auto minmax_y = std::minmax({ triangle[0].y, triangle[1].y, triangle[2].y });

      return 
      {
        { minmax_x.first, minmax_y.first },
        { minmax_x.second, minmax_y.second }
      };
    }

    void TerrainBuilder::apply_face(const std::array<std::uint32_t, 3>& face_indices, std::uint32_t model_id)
    {
      using detail::make_integral;

      const auto& face_v0 = vertices_[face_indices[0]];
      const auto& face_v1 = vertices_[face_indices[1]];
      const auto& face_v2 = vertices_[face_indices[2]];

      auto face_triangle = make_triangle(make_integral(face_v0.vertex.position),
                                         make_integral(face_v1.vertex.position),
                                         make_integral(face_v2.vertex.position));

      if (face_triangle[0] != face_triangle[1] && 
          face_triangle[0] != face_triangle[2] && 
          face_triangle[1] != face_triangle[2])
      {
        query_buffer_.clear();
        insertion_buffer_.clear();
        edge_buffer_.clear();
        intersection_buffer_.clear();

        geometry_.query(boost::geometry::index::intersects(make_triangle_box(face_triangle)),
                        std::back_inserter(query_buffer_));

        // First, find all edges that we're going to have to test for intersections.
        for (const auto& entry : query_buffer_)
        {
          auto face = entry.second;
          std::sort(face.begin(), face.end());

          edge_buffer_.insert(edge_buffer_.end(), {
            { face[0], face[1] },
            { face[0], face[2] },
            { face[1], face[2] }
          });
        }

        // Then, remove all duplicate entries
        std::sort(edge_buffer_.begin(), edge_buffer_.end());
        edge_buffer_.erase(std::unique(edge_buffer_.begin(), edge_buffer_.end()), edge_buffer_.end());

        const std::array<std::uint8_t, 3> edge_indices[] =
        {
          { 0, 1, 2 },
          { 0, 2, 1 },
          { 1, 2, 0 }
        };

        using detail::EdgeIntersection;

        for (const auto edge : edge_buffer_)
        {
          const auto& v0 = vertices_[edge.first];
          const auto& v1 = vertices_[edge.second];

          for (auto face_edge : edge_indices)
          {
            const auto b0 = face_edge[0], b1 = face_edge[1];

            if (auto intersection = find_line_segment_intersection(v0.integral_point, v1.integral_point,
                                                                   face_triangle[b0], face_triangle[b1]))
            {
              EdgeIntersection entry;
              entry.edges = { edge.first, edge.second, b0, b1 };
              entry.point = intersection.point;
              intersection_buffer_.push_back(entry);
            }
          }
        }
        
        using detail::interpolate_vertex;
        for (auto& entry : intersection_buffer_)
        {
          auto a = entry.edges[0], b = entry.edges[1];

          entry.vertex_index = static_cast<std::uint32_t>(vertices_.size());
          vertices_.push_back(interpolate_vertex(entry.point, vertices_[a], vertices_[b]));          
        }

        insertion_buffer_.push_back({
          make_triangle_box(face_triangle),
          { face_indices[0], face_indices[1], face_indices[2], model_id }
        });

        for (const auto& entry : query_buffer_)
        {
          const auto mapped_face = entry.second;
          const auto mapped_model_id = mapped_face[1];

          const auto& mapped_v0 = vertices_[mapped_face[0]];
          const auto& mapped_v1 = vertices_[mapped_face[1]];
          const auto& mapped_v2 = vertices_[mapped_face[2]];

          const auto mapped_triangle = make_triangle(mapped_v0.integral_point,
                                                     mapped_v1.integral_point,
                                                     mapped_v2.integral_point);

          const bool contained_points[] =
          {
            triangle_contains(mapped_triangle, face_triangle[0]),
            triangle_contains(mapped_triangle, face_triangle[1]),
            triangle_contains(mapped_triangle, face_triangle[2]),
          };

          const bool overlapped_points[] =
          {
            triangle_contains_inclusive(face_triangle, mapped_triangle[0]),
            triangle_contains_inclusive(face_triangle, mapped_triangle[1]),
            triangle_contains_inclusive(face_triangle, mapped_triangle[2])
          };

          // If the face to be added is at least partially inside the mapped face, we need to split the mapped face into 
          // smaller triangles in such a way that the face is cut from the geometry.  

          auto same_side = [](auto a, auto b, auto p1, auto p2)
          {
            auto d = b - a;
            auto d1 = d.x * (p1.y - a.y) - d.y * (p1.x - a.x);
            auto d2 = d.x * (p2.y - a.y) - d.y * (p2.x - a.x);

            return d1 * d2 >= 0;
          };

          const bool fully_contained = contained_points[0] && contained_points[1] && contained_points[2];
          const bool fully_overlapped = overlapped_points[0] && overlapped_points[1] && overlapped_points[2];

          boost::container::small_vector<EdgeIntersection, 8> intersections;
          if (!fully_contained && !fully_overlapped)
          {
            // Copy the intersection entries that are a part of the mapped face.
            for (auto edge : edge_indices)
            {
              auto mapped_edge = std::make_pair(mapped_face[edge[0]], mapped_face[edge[1]]);
              if (mapped_edge.first > mapped_edge.second) std::swap(mapped_edge.first, mapped_edge.second);

              auto range_start = std::lower_bound(intersection_buffer_.begin(), intersection_buffer_.end(), mapped_edge,
                                                  [](const EdgeIntersection& e, decltype(mapped_edge) edge)
              {
                return std::make_pair(e.edges[0], e.edges[1]) < edge;
              });

              auto range_end = std::find_if(range_start, intersection_buffer_.end(), [=](const EdgeIntersection& e)
              {
                return std::make_pair(e.edges[0], e.edges[1]) != mapped_edge;
              });

              intersections.insert(intersections.end(), range_start, range_end);
            }
          }

          auto pred = [](auto b) { return b; };
          if (intersections.size() >= 2 || fully_contained || fully_overlapped)
          {
            geometry_.remove(entry);
          }
        }

        geometry_.insert(insertion_buffer_.front());
      }
    }

    TerrainModel TerrainBuilder::build_model(const ElevationMap& elevation_map) const
    {
      TerrainModel model;
      
      for (auto v : vertices_)
      {
        v.vertex.position.z = interpolate_elevation_at(elevation_map, make_2d(v.vertex.position));        
        v.vertex.normal = interpolate_terrain_normal_at(elevation_map, make_2d(v.vertex.position));        

        model.vertices.push_back(v.vertex);
      }

      struct FaceBufferEntry
      {
        Face face;
        std::size_t model_tag;
      };

      std::vector<FaceBufferEntry> face_buffer(geometry_.size());
      std::transform(geometry_.begin(), geometry_.end(), face_buffer.begin(),
                     [this](const auto& entry) -> FaceBufferEntry
      {
        auto& indices = entry.second;

        auto model_id = indices[3];
        auto tag = model_info_[model_id].model_tag;

        Face face = { indices[0], indices[1], indices[2] };        
        return{ face, tag };
      });
      
      std::sort(face_buffer.begin(), face_buffer.end(), [](const auto& a, const auto& b)
      {
        return a.model_tag < b.model_tag;
      });

      for (auto begin = face_buffer.begin(), it = begin, end = face_buffer.end(); it != end; )
      {
        auto range_end = std::find_if(std::next(it), end, [=](const auto& e)
        {
          return it->model_tag != e.model_tag;
        });

        TerrainModel::Component component;
        component.face_index = static_cast<std::uint32_t>(std::distance(begin, it));
        component.face_count = static_cast<std::uint32_t>(std::distance(it, range_end));
        component.model_tag = it->model_tag;
        model.components.push_back(component);

        it = range_end;
      }

      model.faces.resize(face_buffer.size());
      std::transform(face_buffer.begin(), face_buffer.end(), model.faces.begin(), [](const auto& f)
      {
        return f.face;
      });

      return model;
    }
  }
}