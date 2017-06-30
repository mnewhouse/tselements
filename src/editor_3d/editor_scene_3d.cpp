/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "editor_scene_3d.hpp"

namespace ts
{
  namespace editor3d
  {
    EditorScene::EditorScene(resources3d::Track track)
      : track_(std::move(track)),
        render_scene_(track_)
    {
    }

    void EditorScene::render(const scene3d::Viewport& viewport, Vector2i screen_size, double frame_progress) const
    {
      render_scene_.render(viewport, screen_size, frame_progress);
    }

    const resources3d::Track& EditorScene::track() const
    {
      return track_;
    }

    const resources3d::ElevationMap& EditorScene::elevation_map() const
    {
      return track_.elevation_map();
    }

    resources3d::PathLayer* EditorScene::create_path_layer()
    {
      return track_.create_path_layer();
    }

    void EditorScene::commit_path(const resources3d::PathLayer* path_layer, std::uint32_t path_index)
    {
      render_scene_.load_terrain_geometry(track_);
    }

    void EditorScene::update_terrain()
    {
      render_scene_.load_terrain_geometry(track_);
    }
  }
}