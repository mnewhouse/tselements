/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "editor_scene.hpp"

#include "scene/track_scene_generator.hpp"

#include <utility/vector3.hpp>

#include <array>

namespace ts
{
  namespace editor
  {
    const Colorf editor_bg_color = { 0.5f, 0.5f, 0.5f, 1.0f };

    EditorScene::EditorScene(resources::Track track)
      : track_(std::move(track)),
        render_scene_(scene::generate_track_scene(track_))
    {
      render_scene_->set_background_color(editor_bg_color);
    }

    const resources::Track& EditorScene::track() const
    {
      return track_;
    }

    resources::Track& EditorScene::track()
    {
      return track_;
    }

    void EditorScene::render(const scene::Viewport& view_port, Vector2i screen_size, double frame_progress) const
    {
      if (render_scene_)
      {
        render_scene_->render(view_port, screen_size, frame_progress);
      }      
    }

    scene::RenderScene EditorScene::steal_render_scene()
    {
      auto render_scene = std::move(*render_scene_);
      render_scene_ = boost::none;

      return render_scene;
    }

    void EditorScene::adopt_render_scene(scene::RenderScene render_scene)
    {

      render_scene_ = boost::none;
      render_scene_.emplace(std::move(render_scene));

      render_scene_->set_background_color(editor_bg_color);
      render_scene_->clear_dynamic_state();
    }
  }
}