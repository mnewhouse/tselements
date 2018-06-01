/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resources_3d/track_3d.hpp"
#include "scene_3d/render_scene_3d.hpp"

namespace ts
{
  namespace scene3d
  {
    class Viewport;
  }

  namespace editor3d
  {
    class EditorScene
    {
    public:
      explicit EditorScene(resources3d::Track track);

      void render(const scene3d::Viewport& viewport, Vector2i screen_size, double frame_progress) const;

      const resources3d::Track& track() const;
      const resources3d::ElevationMap& elevation_map() const;

      resources3d::PathLayer* create_path_layer();
      void commit_path(const resources3d::PathLayer* path_layer, std::uint32_t path_index);

      void update_terrain();

    private:
      resources3d::Track track_;
      scene3d::RenderScene render_scene_;
    };
  }
}