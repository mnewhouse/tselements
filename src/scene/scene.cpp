/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "scene.hpp"
#include "track_scene.hpp"
#include "dynamic_scene.hpp"
#include "particle_generator.hpp"
#include "scene_renderer.hpp"
#include "car_sound_controller.hpp"
#include "sound_effect_controller.hpp"

namespace ts
{
  namespace scene
  {
    Scene::Scene() = default;
    Scene::~Scene() = default;

    Scene::Scene(Scene&&) = default;
    Scene& Scene::operator=(Scene&&) = default;

    void update_stored_state(Scene& scene)
    {
      scene.dynamic_scene->update_entity_positions();
    }

    void update_scene(Scene& scene, std::uint32_t frame_duration)
    {
      scene.scene_renderer.update(frame_duration);
      scene.car_sound_controller->update(frame_duration);
      scene.particle_generator->update(frame_duration);
    }
  }
}