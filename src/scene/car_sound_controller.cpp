/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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
      playback_properties.volume = 0.0f;
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

    void CarSoundController::pause()
    {
      engine_playback_controller_.pause_all();
      skid_playback_controller_.pause_all();
    }

    void CarSoundController::resume()
    {
      engine_playback_controller_.resume_all();
      skid_playback_controller_.resume_all();
    }

    void CarSoundController::update(std::uint32_t frame_duration)
    {
      for (auto& car_info : cars_)
      {
        const auto& car = *car_info.car;
        const auto& handling_state = car.handling_state();
        auto rev_speed = handling_state.engine_rev_speed;

        if (car_info.engine_sound)
        {
          auto pitch = std::min(0.3 + rev_speed * 0.7, 1.1);
          auto throttle_rate = car.control_state(controls::Control::Throttle) / 255.0;

          auto frame_increment = frame_duration * 0.001 * 0.5;

          auto volume = std::max(static_cast<double>(car_info.engine_sound.volume()), 0.3);
          auto target_volume = 0.3 + 0.3 * rev_speed + 0.4 * throttle_rate;

          if (target_volume > volume)
          {
            volume = std::min(volume + frame_increment, target_volume);
          }

          else
          {
            volume = std::max(volume - frame_increment, target_volume);
          }

          car_info.engine_sound.set_volume(static_cast<float>(std::min(volume, 1.0)));
          car_info.engine_sound.set_pitch(static_cast<float>(std::min(pitch, 1.1)));
        }

        
        auto max_skid_magnitude = 0.0;
        for (auto& ws : handling_state.wheel_states)
        {
          if (ws.terrain_roughness < 0.001)
          {
            max_skid_magnitude = std::max(max_skid_magnitude, ws.speed * ws.slide_ratio);
          }          
        }

        if (max_skid_magnitude >= 1.0)
        {          
          auto volume = std::min(max_skid_magnitude * 0.006, 1.0);

          if (!car_info.skid_sound)
          {
            audio::PlaybackProperties properties;
            car_info.skid_sound = skid_playback_controller_.play_looped_sound_effect(*skid_sound_, properties, 1);
          }

          if (car_info.skid_sound)
          {
            car_info.skid_sound.set_volume(static_cast<float>(volume * volume * 0.5));
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