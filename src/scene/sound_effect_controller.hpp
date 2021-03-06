/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "audio/sound_sample_loader.hpp"
#include "audio/sound_playback_controller.hpp"

namespace ts
{
  namespace world
  {
    class Entity;
    struct CollisionResult;
  }

  namespace scene
  {
    // The SoundEffectController is responsible for playing collision sounds and other types
    // of effects.
    class SoundEffectController
    {
    public:
      explicit SoundEffectController(std::size_t num_channels);

      void play_collision_sound(const world::Entity& entity, const world::CollisionResult& collision_result);

      void play_collision_sound(const world::Entity& subject, const world::Entity& object,
                                const world::CollisionResult& collision_result);

      void pause_all();
      void resume_all();

    private:
      audio::PlaybackController playback_controller_;
      audio::SoundSampleLoader sound_sample_bank_;

      const audio::SoundSample* scenery_collision_sample_;
      const audio::SoundSample* entity_collision_sample_;
    };
  }
}
