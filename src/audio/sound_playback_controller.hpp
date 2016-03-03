/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SOUND_PLAYBACK_CONTROLLER_HPP_5891182931
#define SOUND_PLAYBACK_CONTROLLER_HPP_5891182931

#include "sound_sample.hpp"

#include <SFML/Audio/Sound.hpp>

#include <cstddef>

namespace ts
{
  namespace audio
  {
    struct PlaybackProperties
    {
      float volume = 1.0f; // 0.0-1.0
      float pitch = 1.0f;
      float attenuation = 1.0f;
      float min_distance = 0.0f;
      bool relative_to_listener = false;
    };

    class PlaybackController;

    class PlaybackHandle
    {
    public:
      PlaybackHandle() = default;

      void set_volume(float volume);
      float volume() const;

      void set_pitch(float pitch);
      float pitch() const;

      void set_attenuation(float attenuation);
      float attenuation() const;

      explicit operator bool() const;

    private:
      explicit PlaybackHandle(sf::Sound* sound);

      friend PlaybackController;
      sf::Sound* sound_ = nullptr;
    };

    class PlaybackController
    {
    public:
      explicit PlaybackController(std::size_t num_channels);

      void play_sound_effect(const SoundSample& sample, const PlaybackProperties& properties,
                                std::size_t priority);

      PlaybackHandle play_looped_sound_effect(const SoundSample& sample, const PlaybackProperties& properties,
                                              std::size_t priority);

      void stop_sound_playback(PlaybackHandle playback_handle);
      void set_sound_priority(PlaybackHandle playback_handle, std::size_t priority);

    private:
      PlaybackHandle play_sound_effect_internal(const SoundSample& sample, const PlaybackProperties& properties,
                                                std::size_t priority, bool looped);

      struct SoundInfo
      {
        sf::Sound* sound;
        std::size_t priority;

        struct PriorityCmp;
      };

      std::vector<SoundInfo> active_sounds_;
      std::vector<sf::Sound> sounds_;
    };
  }
}

#endif