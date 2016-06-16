/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "editor_scene.hpp"

#include <utility/vector3.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <array>

namespace ts
{
  namespace editor
  {
    namespace detail
    {
      boost::optional<std::pair<float, Vector3f>>
        triangle_ray_intersection(const std::array<glm::vec3, 3>& triangle,
                                  glm::vec3 ray_start, glm::vec3 ray_end)
      {
        auto u = triangle[1] - triangle[0];
        auto v = triangle[2] - triangle[0];
        auto normal = normalize(glm::cross(u, v));
        auto ray_direction = ray_end - ray_start;

        auto ray_projection = glm::dot(normal, ray_direction);
        if (std::abs(ray_projection) >= 0.00001f)
        {
          auto intersect_point_1d = glm::dot(normal, triangle[0] - ray_start) / ray_projection;
          if (intersect_point_1d >= 0.0f && intersect_point_1d <= 1.0f)
          {
            auto intersect_point = ray_start + intersect_point_1d * ray_direction;

            auto uu = glm::dot(u, u);
            auto uv = glm::dot(u, v);
            auto vv = glm::dot(v, v);
            auto w = intersect_point - triangle[0];
            auto wu = dot(w, u);
            auto wv = dot(w, v);
            auto d = uv * uv - uu * vv;

            auto s = (uv * wv - vv * wu) / d;
            if (s >= 0.0 && s <= 1.0)
            {
              auto t = (uv * wu - uu * wv) / d;
              if (t >= 0.0 && s + t <= 1.0)
              {
                Vector3f result = { intersect_point.x, intersect_point.y, intersect_point.z };
                return std::make_pair(intersect_point_1d, result);
              }
            }
          }
        }

        return boost::none;
      };
    }

    EditorScene::EditorScene(resources_3d::Track track)
      : track_(std::move(track)) 
    {
      auto paths = track_.paths();
      if (!paths.empty())
      {
        selected_track_path_ = paths.front();
      }
    }

    void EditorScene::render() const
    {
      render_scene_.render(screen_size_, view_port_);
    }

    void EditorScene::load_scene()
    {
      render_scene_.load_track_visuals(track_);
    }

    const scene_3d::RenderScene& EditorScene::render_scene() const
    {
      return render_scene_;
    }

    void EditorScene::move_camera(Vector3f offset)
    {
      render_scene_.move_camera(offset);
    }

    void EditorScene::move_camera_2d(Vector2f offset)
    {
      render_scene_.move_camera_2d(offset, track_.height_map());
    }

    resources_3d::TrackPath* EditorScene::create_track_path()
    {
      auto path = track_.create_path();
      render_scene_.register_track_path(path);
      return path;
    }

    resources_3d::TrackPath* EditorScene::selected_track_path()
    {
      return selected_track_path_;
    }

    const resources_3d::TrackPath* EditorScene::selected_track_path() const
    {
      return selected_track_path_;
    }

    std::size_t EditorScene::selected_track_path_stroke_index() const
    {
      return selected_track_path_stroke_index_;
    }
    
    void EditorScene::select_track_path_stroke_index(std::size_t index)
    {
      selected_track_path_stroke_index_ = index;
    }

    void EditorScene::select_track_path(resources_3d::TrackPath* path)
    {
      selected_track_path_ = path;
    }

    void EditorScene::commit(const resources_3d::TrackPath* track_path)
    {
      render_scene_.update(track_path);
    }

    void EditorScene::commit(const resources_3d::TrackPath* track_path,
                             std::size_t node_index, std::size_t node_count)
    {
      render_scene_.update(track_path, node_index, node_count);
    }

    // Relative_pos.xy are in range [0-1]
    boost::optional<Vector3f> EditorScene::screen_to_terrain_position(Vector2i absolute_pos) const
    {
      if (screen_size_.x == 0 || screen_size_.y == 0) return boost::none;

      // Need to get the relative position *in the viewport*.      
      Vector2f relative_pos;
      {
        auto screen_size = vector2_cast<std::int32_t>(screen_size_);
        auto left = view_port_.left - (screen_size.x - view_port_.width) / 2;
        auto bottom = (screen_size.y - view_port_.height) -
          view_port_.top - (screen_size.y - view_port_.height) / 2;

        absolute_pos.y = screen_size.y - absolute_pos.y;
        relative_pos = vector2_cast<float>((absolute_pos - make_vector2(left, bottom))) /
          (vector2_cast<float>(screen_size) * 0.5f) - 1.0f;
      }

      auto view = render_scene_.view_matrix();
      auto projection = render_scene_.projection_matrix();

     auto camera_position = render_scene_.camera_position();
     auto track_size = track_.size();

     auto projected_view = projection * view;
     auto inverse_view = inverse(projected_view);

     auto intermediate_z = inverse_view[0].z * relative_pos.x + 
       inverse_view[1].z * relative_pos.y + inverse_view[3].z;
     auto intermediate_w = inverse_view[0].w * relative_pos.y + 
       inverse_view[1].w * relative_pos.y + inverse_view[3].w;

     auto calculate_projection_z_coord = [&](float world_z)
     {
       return (intermediate_z - (intermediate_w * world_z)) / (-inverse_view[2].z + world_z * inverse_view[2].w);
     };

     auto calculate_world_coords_from_projection_z = [&](float projection_z)
     {
       auto vec = inverse_view * glm::vec4(relative_pos.x, relative_pos.y, projection_z, 1.0f);
       return vec /= vec.w;
     };

     auto track_height = static_cast<float>(track_size.z);
     auto bounds = std::make_pair(calculate_projection_z_coord(0.0f),
                                  calculate_projection_z_coord(track_height));

     auto ray_start_4d = inverse_view * glm::vec4(relative_pos.x, relative_pos.y, bounds.first, 1.0f);
     auto ray_end_4d = inverse_view * glm::vec4(relative_pos.x, relative_pos.y, bounds.second, 1.0f);

     ray_start_4d /= ray_start_4d.w;
     ray_end_4d /= ray_end_4d.w;

     auto ray_start = glm::vec3(ray_start_4d);
     auto ray_end = glm::vec3(ray_end_4d);

     const auto& height_map = track_.height_map();
     const std::int32_t cell_size = height_map.cell_size();

     auto world_bounds = make_vector2(static_cast<float>(track_size.x), static_cast<float>(track_size.y));

     auto bounding_box = intersection(make_rect_from_points(make_vector2(ray_start.x, ray_start.y), 
                                                            make_vector2(ray_end.x, ray_end.y)),
                                      make_rect_from_points(make_vector2(0.0f, 0.0f), world_bounds));

     if (bounding_box.width > 0.0f && bounding_box.height > 0.0f)
     {
       IntRect map_bounds;
       map_bounds.left = static_cast<std::int32_t>(bounding_box.left) / cell_size;
       map_bounds.top = static_cast<std::int32_t>(bounding_box.top) / cell_size;
       map_bounds.width = static_cast<std::int32_t>(bounding_box.right()) / cell_size + 1 - map_bounds.left;
       map_bounds.height = static_cast<std::int32_t>(bounding_box.bottom()) / cell_size + 1 - map_bounds.top;

       boost::optional<std::pair<float, Vector3f>> result;

       // Loop through all height map cells that intersect with the potential area.
       std::size_t idx = 0;
       for (auto y = map_bounds.top, bottom = map_bounds.bottom(); y != bottom; ++y)
       {
         for (auto x = map_bounds.left, right = map_bounds.right(); x != right; ++x)
         {
           // Then, see if the screen position is inside the height map cell.
           auto top_left = make_vector2(static_cast<float>(x),
                                        static_cast<float>(y));
           auto bottom_right = top_left + 1.0f;

           top_left *= static_cast<float>(cell_size);
           bottom_right *= static_cast<float>(cell_size);

           auto cell = make_rect_from_points(top_left, bottom_right);

           std::array<glm::vec3, 3> first_triangle =
           {
             {
               { cell.left, cell.top, height_map(x, y) },
               { cell.right(), cell.top, height_map(x + 1, y) },
               { cell.left, cell.bottom(), height_map(x, y + 1) }
             }
           };

           std::array<glm::vec3, 3> second_triangle =
           {
             {
               { cell.right(), cell.top, height_map(x + 1, y) },
               { cell.left, cell.bottom(), height_map(x, y + 1) },
               { cell.right(), cell.bottom(), height_map(x + 1, y + 1)}
             }
           };

           auto first_intersection = detail::triangle_ray_intersection(first_triangle,
                                                                       ray_start,
                                                                       ray_end);

           auto second_intersection = detail::triangle_ray_intersection(second_triangle,
                                                                        ray_start,
                                                                        ray_end);

           auto cmp = [](const auto& a, const auto& b)
           {
             if (!a) return false;
             if (!b) return true;

             return a->first < b->first;
           };

           const auto& intersection = std::min(first_intersection, second_intersection, cmp);
           if (cmp(intersection, result))
           {
             result = intersection;
           }
         }
       }

       if (result) return result->second;
     }

     return boost::none;
    }

    // This function gets the terrain elevation at the specified 2D position,
    // and uses that as its Z value.
    Vector2i EditorScene::world_to_screen_position(Vector2f position) const
    {
      const auto& height_map = track_.height_map();

      Vector3f position_3d(position.x, position.y, interpolate_height_at(height_map, position));
      return world_to_screen_position(position_3d);
    }

    Vector2i EditorScene::world_to_screen_position(Vector3f position) const
    {
      auto view = render_scene_.view_matrix();
      auto projection = render_scene_.projection_matrix();

      auto projected_view = projection * view;
      auto relative_pos = projected_view * glm::vec4(position.x, position.y, position.z, 1.0);
      relative_pos /= relative_pos.w;

      // Transform relative position according to viewport

      auto screen_size = vector2_cast<std::int32_t>(screen_size_);
      auto view_port = view_port_;

      auto left = view_port.left - (screen_size.x - view_port.width) / 2;
      auto bottom = (screen_size.y - view_port.height) -
        view_port.top - (screen_size.y - view_port.height) / 2;     
      
      auto x = (relative_pos.x + 1.0f) * (screen_size.x / 2) + left;
      auto y = (relative_pos.y + 1.0f) * (screen_size.y / 2) + bottom;
      return make_vector2(static_cast<std::int32_t>(x),
                          static_cast<std::int32_t>(screen_size.y - y));
    }

    void EditorScene::set_view_port(Vector2u screen_size, IntRect view_port)
    {
      screen_size_ = screen_size;
      view_port_ = view_port;
    }

    IntRect EditorScene::view_port() const
    {
      return view_port_;
    }

    Vector2u EditorScene::screen_size() const
    {
      return screen_size_;
    }
  }
}