/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef EDITOR_SCENE_HPP_85918214
#define EDITOR_SCENE_HPP_85918214

#include "resources/track.hpp"

#include "scene/render_scene.hpp"

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
      explicit EditorScene(resources::Track track);

      const resources::Track& track() const;
      resources::Track& track();

      void render(const scene::Viewport& viewport, Vector2u screen_size, double frame_progress) const;

      scene::RenderScene steal_render_scene();
      void adopt_render_scene(scene::RenderScene render_scene);

    private:
      resources::Track track_;
      boost::optional<scene::RenderScene> render_scene_;
    };
  }
}

#endif