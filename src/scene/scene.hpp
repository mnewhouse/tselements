/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SCENE_HPP_442903
#define SCENE_HPP_442903

#include <memory>

#include "scene_renderer.hpp"

namespace ts
{
  namespace stage
  {
    class Stage;
  }

  namespace scene
  {
    class TrackScene;
    class DynamicScene;
    class ParticleGenerator;
    class CarSoundController;
    class SoundEffectController;

    // The Scene represents all objects that are needed to deliver the client-sided
    // part of the game experience. This basically just means video and audio.
    struct Scene
    {
      Scene();
      ~Scene();

      Scene(Scene&&);
      Scene& operator=(Scene&&);

      const stage::Stage* stage_ptr;

      std::unique_ptr<TrackScene> track_scene;
      std::unique_ptr<DynamicScene> dynamic_scene;
      std::unique_ptr<ParticleGenerator> particle_generator;

      std::unique_ptr<CarSoundController> car_sound_controller;
      std::unique_ptr<SoundEffectController> sound_effect_controller;

      SceneRenderer scene_renderer;
    };

    void update_stored_state(Scene& scene_obj);
    void update_scene(Scene& scene_obj, std::uint32_t frame_duration);
  }
}

#endif