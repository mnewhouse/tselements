/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SCENE_HPP_442903
#define SCENE_HPP_442903

#include <memory>

#include "track_scene.hpp"
#include "dynamic_scene.hpp"
#include "particle_generator.hpp"
#include "scene_renderer.hpp"
#include "car_sound_controller.hpp"

namespace ts
{
  namespace stage
  {
    class Stage;
  }

  namespace scene
  {
    // The Scene represents all objects that are needed to deliver the client-sided
    // part of the game experience. This basically just means video and audio.
    struct Scene
    {
      const stage::Stage* stage_ptr;

      std::unique_ptr<TrackScene> track_scene;
      std::unique_ptr<DynamicScene> dynamic_scene;
      std::unique_ptr<ParticleGenerator> particle_generator;

      SceneRenderer scene_renderer;

      CarSoundController car_sound_controller;
    };   
  }
}

#endif