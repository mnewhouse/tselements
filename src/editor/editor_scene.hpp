/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resources/track.hpp"

#include "scene/render_scene.hpp"

#include "utility/vector2.hpp"
#include "utility/vector3.hpp"

#include <boost/optional.hpp>

#include <vector>

namespace ts
{
  namespace editor
  {
    class EditorScene
    {
    public:
      explicit EditorScene(resources::Track track);

      const resources::Track& track() const;
      resources::Track& track();

      using render_callback = scene::RenderScene::render_callback;

      void render(const scene::Viewport& viewport, Vector2i screen_size, double frame_progress,
                  const render_callback& post_render = nullptr) const;

      scene::RenderScene steal_render_scene();
      void adopt_render_scene(scene::RenderScene render_scene);

      const scene::RenderScene* render_scene() const;

      void append_tile(resources::TrackLayer* track_layer, const resources::Tile& tile);

    private:
      const std::vector<resources::PlacedTile>& expand_tile(const resources::Tile& tile) const;

      resources::Track track_;
      boost::optional<scene::RenderScene> render_scene_;

      mutable std::vector<resources::PlacedTile> tile_expansion_cache_;
    };
  }
}