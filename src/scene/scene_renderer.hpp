/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SCENE_RENDERER_HPP_5102093190
#define SCENE_RENDERER_HPP_5102093190

#include "drawable_entity.hpp"

#include "utility/vector2.hpp"

#include <vector>

namespace ts
{
  namespace scene
  {
    class DynamicScene;
    class TrackScene;
    class ParticleGenerator;
    class Viewport;

    class SceneRenderer
    {
    public:
      // Construct a SceneRenderer with scene pointers. It stores raw pointers to these objects,
      // so the caller must ensure that these stay alive.
      SceneRenderer();
      explicit SceneRenderer(const TrackScene* track_scene, const DynamicScene* dynamic_scene,
                             const ParticleGenerator* particle_generator);
      ~SceneRenderer();

      SceneRenderer(SceneRenderer&&);
      SceneRenderer& operator=(SceneRenderer&&);

      void render(const Viewport& viewport, Vector2u screen_size, double frame_progress);
      void update(std::uint32_t frame_duration);


    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}

#endif