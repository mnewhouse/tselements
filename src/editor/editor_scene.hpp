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

      Vector2u screen_size() const;
      IntRect view_port() const;
      void set_view_port(Vector2u screen_size, IntRect view_port);

      void render() const;

      void move_camera(Vector3f offset);
      void move_camera_2d(Vector2f offset);

      const scene_3d::RenderScene& render_scene() const;

      boost::optional<Vector3f> screen_to_terrain_position(Vector2i pos) const;

      resources_3d::TrackPath* create_track_path();
      resources_3d::TrackPath* selected_track_path();
      const resources_3d::TrackPath* selected_track_path() const;
      void select_track_path(resources_3d::TrackPath* path);

      void commit(const resources_3d::TrackPath* track_path);
      void commit(const resources_3d::TrackPath* track_path,
                  std::size_t node_index, std::size_t node_count);

    private:
      resources_3d::Track track_;
      scene_3d::RenderScene render_scene_;

      resources_3d::TrackPath* selected_track_path_ = nullptr;

      Vector2u screen_size_;
      IntRect view_port_;
    };
  }
}

#endif