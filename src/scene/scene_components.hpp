/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "dynamic_scene.hpp"
#include "particle_generator.hpp"
#include "car_sound_controller.hpp"
#include "sound_effect_controller.hpp"

namespace ts
{
  namespace stage
  {
    class Stage;
  }

  namespace scene
  {
    struct SceneComponents
    {
      const stage::Stage* stage_ptr_;

      DynamicScene dynamic_scene_;
      ParticleGenerator particle_generator_;

      CarSoundController car_sound_controller_;
      SoundEffectController sound_effect_controller_;
    };
  }
}
