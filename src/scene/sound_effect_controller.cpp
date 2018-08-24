/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


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

    void SoundEffectController::play_collision_sound(const world::Entity& entity,
                                                     const world::CollisionResult& collision_result)
    {
      /*
      if (scenery_collision_sample_)
      {
        audio::PlaybackProperties properties;

        auto impact = static_cast<float>(collision_result.impact * collision_result.bounce_factor * 0.005);
        properties.volume = std::min(impact, 1.0f);

        playback_controller_.play_sound_effect(*scenery_collision_sample_, properties, 1);
      }*/
    }

    void SoundEffectController::play_collision_sound(const world::Entity& subject, const world::Entity& object,
                                                     const world::CollisionResult& collision_result)
    {
      /*
      if (entity_collision_sample_)
      {
        audio::PlaybackProperties properties;

        auto impact = static_cast<float>(collision_result.impact * collision_result.bounce_factor * 0.005);
        properties.volume = std::min(impact, 1.0f);

        playback_controller_.play_sound_effect(*entity_collision_sample_, properties, 1);
      }
      */
    }

    void SoundEffectController::pause_all()
    {
      playback_controller_.pause_all();
    }

    void SoundEffectController::resume_all()
    {
      playback_controller_.resume_all();
    }
  }
}
