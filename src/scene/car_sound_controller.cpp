/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "car_sound_controller.hpp"

#include "resources/car_definition.hpp"

#include "world/car.hpp"
#include "world/world_limits.hpp"

#include <algorithm>

namespace ts
{
  namespace scene
  {
    CarSoundController::CarSoundController()
      : engine_playback_controller_(8),
        skid_playback_controller_(4),
        skid_sound_(sound_bank_.load_sound_effect("sound/skid.wav", std::nothrow))
    {
      cars_.reserve(world::limits::max_car_count);
    }

    std::size_t CarSoundController::register_car_model(const resources::CarDefinition& car_def)
    {
      std::size_t model_id = car_models_.size();
      const auto& sound = sound_bank_.load_sound_effect(car_def.engine_sound_path);
      
      car_models_.emplace_back();
      car_models_.back().engine_sound = &sound;
      return model_id;
    }

    void CarSoundController::register_car(std::size_t model_id, const world::Car* car)
    {
      const auto* sound_sample = car_models_[model_id].engine_sound;

      audio::PlaybackProperties playback_properties;
      playback_properties.pitch = 0.5f;
      auto sound = engine_playback_controller_.play_looped_sound_effect(*sound_sample, playback_properties, 1);

      Car car_info;
      car_info.model_id = model_id;
      car_info.car = car;
      car_info.engine_sound = sound;

      cars_.push_back(car_info);
    }

    void CarSoundController::unregister_car(const world::Car* car)
    {
      auto it = std::find_if(cars_.begin(), cars_.end(), 
                             [=](const auto& entry)
      {
        return entry.car == car;
      });

      if (it != cars_.end())
      {
        engine_playback_controller_.stop_sound_playback(it->engine_sound);
        skid_playback_controller_.stop_sound_playback(it->skid_sound);
        cars_.erase(it);
      }
    }

    void CarSoundController::update(std::uint32_t frame_duration)
    {
      for (auto& car_info : cars_)
      {
        if (car_info.engine_sound)
        {
          auto pitch = static_cast<float>(0.25 + car_info.car->engine_rev_speed() * 0.75);

          car_info.engine_sound.set_volume(std::min(pitch * pitch, 1.0f));
          car_info.engine_sound.set_pitch(std::min(pitch, 1.1f));
        }

        if (car_info.car->traction() < 0.95)
        {
          auto velocity = car_info.car->velocity();
          auto speed = magnitude(velocity);
          auto volume = static_cast<float>(speed / (400.0 * std::max(car_info.car->traction(), 0.25)));
          volume = std::min(volume, 1.0f);

          if (!car_info.skid_sound)
          {
            audio::PlaybackProperties properties;
            car_info.skid_sound = skid_playback_controller_.play_looped_sound_effect(*skid_sound_, properties, 1);
          }

          if (car_info.skid_sound)
          {
            car_info.skid_sound.set_volume(volume * volume * 0.6f);
          }          
        }

        else if (car_info.skid_sound)
        {
          skid_playback_controller_.stop_sound_playback(car_info.skid_sound);
          car_info.skid_sound = {};
        }
      }
    }
  }
}