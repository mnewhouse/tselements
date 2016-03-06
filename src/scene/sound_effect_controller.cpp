/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SOUND_EFFECT_CONTROLLER_HPP_4419812985
#define SOUND_EFFECT_CONTROLLER_HPP_4419812985

#include "sound_effect_controller.hpp"

#include "core/config.hpp"

namespace ts
{
  namespace scene
  {
    SoundEffectController::SoundEffectController(std::size_t channel_count)
      : playback_controller_(channel_count),
        sound_sample_bank_()
    {
      std::string audio_directory = config::audio_directory;
      scenery_collision_sample_ = sound_sample_bank_.load_sound_effect(audio_directory + "/collision.wav", std::nothrow);
      entity_collision_sample_ = sound_sample_bank_.load_sound_effect(audio_directory + "/carcollision.wav", std::nothrow);
    }
  }
}

#endif