/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <memory>

#include "utility/vector2.hpp"

#include "world/world_message_fwd.hpp"

namespace ts
{
  namespace stage
  {
    class Stage;
  }

  namespace scene
  {
    struct SceneComponents;
    class RenderScene;
    class ViewportArrangement;

    // The Scene represents all objects that are needed to deliver the client-sided
    // part of the game experience. This basically just means video and audio.
    class Scene
    {
    public:
      Scene();
      Scene(SceneComponents scene_components, RenderScene render_scene);
      ~Scene();

      Scene(Scene&&);
      Scene& operator=(Scene&&);

      const stage::Stage& stage() const;

      void update_stored_state();
      void update(std::uint32_t frame_duration);

      void render(const ViewportArrangement& viewport_arrangement, Vector2i screen_size,
                  double frame_progress) const;

      void handle_collision(const world::messages::SceneryCollision& collision);
      void handle_collision(const world::messages::EntityCollision& collision);

      RenderScene steal_render_scene();

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}
