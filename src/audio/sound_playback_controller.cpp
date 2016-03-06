/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "sound_playback_controller.hpp"

#include <algorithm>

namespace ts
{
  namespace audio
  {
    namespace detail
    {
      void play_sound(sf::Sound& sound, const SoundSample& sample, const PlaybackProperties& properties, bool looped)
      {
        sound.setBuffer(sample);
        sound.setPitch(properties.pitch);
        sound.setAttenuation(properties.attenuation);
        sound.setLoop(looped);
        sound.setRelativeToListener(properties.relative_to_listener);
        sound.setMinDistance(properties.min_distance);
        sound.setVolume(properties.volume * 100.0f);
        sound.play();
      }
    }

    struct PlaybackController::SoundInfo::PriorityCmp
    {
      bool operator()(const SoundInfo& a, const SoundInfo& b) const
      {
        return a.priority > b.priority;
      }
    };

    PlaybackController::PlaybackController(std::size_t num_channels)
      : sounds_(num_channels)
    {
      active_sounds_.reserve(num_channels);
    }

    PlaybackHandle PlaybackController::play_sound_effect_internal(const SoundSample& sample, const PlaybackProperties& properties,
                                                                  std::size_t priority, bool looped)
    {

      // Find an empty sound, or as a fallback, a sound with a lower priority than the specified priority.
      auto idle_sound = std::find_if(sounds_.begin(), sounds_.end(),
                                     [](const sf::Sound& sound)
      {
        return sound.getStatus() == sf::Sound::Stopped;
      });

      if (idle_sound != sounds_.end())
      {
        auto& sound = *idle_sound;
        detail::play_sound(sound, sample, properties, looped);
        return PlaybackHandle(&sound);
      }

      // No idle sounds, we need to replace the sound with the lowest priority with the new one,
      // granted that the new priority is higher.
      auto lowest_priority = std::min_element(active_sounds_.begin(), active_sounds_.end(), SoundInfo::PriorityCmp());
      if (lowest_priority != active_sounds_.end() && lowest_priority->priority < priority)
      {
        auto sound = lowest_priority->sound;
        detail::play_sound(*sound, sample, properties, looped);
        lowest_priority->priority = priority;
        return PlaybackHandle(sound);
      };

      return PlaybackHandle();
    }

    void PlaybackController::play_sound_effect(const SoundSample& sample, const PlaybackProperties& properties,
                                               std::size_t priority)
    {
      play_sound_effect_internal(sample, properties, priority, false);
    }

    PlaybackHandle PlaybackController::play_looped_sound_effect(const SoundSample& sample, const PlaybackProperties& properties,
                                                                std::size_t priority)
    {
      return play_sound_effect_internal(sample, properties, priority, true);
    }

    void PlaybackController::stop_sound_playback(PlaybackHandle handle)
    {
      handle.sound_->stop();
    }

    PlaybackHandle::PlaybackHandle(sf::Sound* sound)
      : sound_(sound)
    {}

    PlaybackHandle::operator bool() const
    {
      return sound_ != nullptr;
    }

    void PlaybackHandle::set_volume(float volume)
    {
      sound_->setVolume(volume * 100.f);
    }

    float PlaybackHandle::volume() const
    {
      return sound_->getVolume() * 0.01f;
    }

    void PlaybackHandle::set_pitch(float pitch)
    {
      sound_->setPitch(pitch);
    }

    float PlaybackHandle::pitch() const
    {
      return sound_->getPitch();
    }

    void PlaybackHandle::set_attenuation(float attenuation)
    {
      sound_->setAttenuation(attenuation);
    }

    float PlaybackHandle::attenuation() const
    {
      return sound_->getAttenuation();
    }
  }
}