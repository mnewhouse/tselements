/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "editor_scene.hpp"

#include <utility/vector3.hpp>

#include <array>

namespace ts
{
  namespace editor
  {
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

    const resources_3d::Track& EditorScene::track() const
    {
      return track_;
    }

    resources_3d::Track& EditorScene::track()
    {
      return track_;
    }

    Vector3f EditorScene::camera_position() const
    {
      return render_scene_.camera_position();
    }

    void EditorScene::move_camera(Vector3f offset)
    {
      render_scene_.move_camera(offset);
    }

    void EditorScene::move_camera_2d(Vector2f offset)
    {
      render_scene_.move_camera(make_3d(offset));
    }

    resources_3d::TrackPath* EditorScene::create_track_path()
    {
      auto path = track_.create_path();
      render_scene_.terrain_scene().register_track_path(path, track_.height_map());
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
      render_scene_.terrain_scene().update(track_path, track_.height_map());
    }

    boost::optional<Vector3f> EditorScene::terrain_position_at(Vector2i screen_position) const
    {
      auto projected_view = render_scene_.projection_matrix() * render_scene_.view_matrix();
      auto screen_size = vector2_cast<std::int32_t>(screen_size_);

      const auto& terrain_scene = render_scene_.terrain_scene();
      return terrain_scene.find_terrain_position_at(screen_position, screen_size,
                                                    view_port_, track_.height_map(), projected_view);
    }

    // This function gets the terrain elevation at the specified 2D position,
    // and uses that as its Z value.
    Vector2i EditorScene::world_to_screen_position(Vector2f position) const
    {
      return world_to_screen_position(make_vector3(position.x, position.y,
                                                   interpolate_height_at(track_.height_map(), position)));
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

    void EditorScene::rebuild_height_map()
    {
      auto size = track_.height_map().size();
      rebuild_height_map(IntRect(0, 0, size.x, size.y));
    }

    void EditorScene::rebuild_height_map(IntRect area)
    {
      render_scene_.terrain_scene().rebuild_height_map(track_.height_map(), area);
    }
  }
}