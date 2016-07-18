/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_SCENE_HPP_85918214
#define EDITOR_SCENE_HPP_85918214

#include "track_3d.hpp"
#include "render_scene_3d.hpp"

#include "utility/vector2.hpp"
#include "utility/vector3.hpp"

#include <boost/optional.hpp>

namespace ts
{
  namespace editor
  {
    class EditorScene
    {
    public:
      explicit EditorScene(resources_3d::Track track);

      void load_scene();

      const resources_3d::Track& track() const;
      resources_3d::Track& track();

      Vector2u screen_size() const;
      IntRect view_port() const;
      void set_view_port(Vector2u screen_size, IntRect view_port);

      void render() const;

      void move_camera(Vector3f offset);
      void move_camera_2d(Vector2f offset);

      const scene_3d::RenderScene& render_scene() const;

      void rebuild_height_map(IntRect updated_area);
      void rebuild_height_map();

      boost::optional<Vector3f> terrain_position_at(Vector2i screen_position) const;

      Vector2i world_to_screen_position(Vector2f position) const;
      Vector2i world_to_screen_position(Vector3f world_position) const;

      resources_3d::TrackPath* create_track_path();
      resources_3d::TrackPath* selected_track_path();
      const resources_3d::TrackPath* selected_track_path() const;
      std::size_t selected_track_path_stroke_index() const;

      void select_track_path(resources_3d::TrackPath* path);
      void select_track_path_stroke_index(std::size_t index);

      void commit(const resources_3d::TrackPath* track_path);
      void commit(const resources_3d::TrackPath* track_path,
                  std::size_t node_index, std::size_t node_count);

      Vector3f camera_position() const;

    private:
      resources_3d::Track track_;
      scene_3d::RenderScene render_scene_;

      resources_3d::TrackPath* selected_track_path_ = nullptr;
      std::size_t selected_track_path_stroke_index_ = 0;

      Vector2u screen_size_;
      IntRect view_port_;
    };
  }
}

#endif