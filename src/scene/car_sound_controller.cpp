/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "car_sound_controller.hpp"

#include "audio/sound_sample.hpp"
#include "audio/sound_sample_loader.hpp"
#include "audio/sound_playback_controller.hpp"

#include "resources/car_definition.hpp"

#include "world/car.hpp"
#include "world/world_limits.hpp"

namespace ts
{
  namespace scene
  {
    namespace detail
    {
    }

    struct CarSoundController::Impl
    {
      struct CarModel
      {
        const audio::SoundSample* engine_sound;
      };

      struct Car
      {
        std::size_t model_id;
        const world::Car* car;

        audio::PlaybackHandle engine_sound;
        audio::PlaybackHandle skid_sound;
      };

      Impl();

      std::vector<Car> cars_;
      std::vector<CarModel> car_models_;
      audio::SoundSampleLoader sound_bank_;
      audio::PlaybackController engine_playback_controller_;
      audio::PlaybackController skid_playback_controller_;

      const audio::SoundSample* skid_sound_;
    };

    CarSoundController::Impl::Impl()
      : engine_playback_controller_(8),
        skid_playback_controller_(4),
        skid_sound_(&sound_bank_.load_sound_effect("sound/skid.wav"))
    {
      cars_.reserve(world::limits::max_car_count);
    }

    CarSoundController::CarSoundController()
      : impl_(std::make_unique<Impl>())
    {}

    CarSoundController::~CarSoundController()
    {
    }

    CarSoundController::CarSoundController(CarSoundController&& other)
      : impl_(std::move(other.impl_))
    {
    }

    CarSoundController& CarSoundController::operator=(CarSoundController&& other)
    {
      impl_ = std::move(other.impl_);
      return *this;
    }

    std::size_t CarSoundController::register_car_model(const resources::CarDefinition& car_def)
    {
      std::size_t model_id = impl_->car_models_.size();
      const auto& sound = impl_->sound_bank_.load_sound_effect(car_def.engine_sound_path);
      
      impl_->car_models_.emplace_back();
      impl_->car_models_.back().engine_sound = &sound;
      return model_id;
    }

    void CarSoundController::register_car(std::size_t model_id, const world::Car* car)
    {
      const auto* sound_sample = impl_->car_models_[model_id].engine_sound;

      audio::PlaybackProperties playback_properties;
      playback_properties.pitch = 0.5f;
      auto sound = impl_->engine_playback_controller_.play_looped_sound_effect(*sound_sample, playback_properties, 1);

      Impl::Car car_info;
      car_info.model_id = model_id;
      car_info.car = car;
      car_info.engine_sound = sound;

      impl_->cars_.push_back(car_info);
    }

    void CarSoundController::unregister_car(const world::Car* car)
    {
      auto it = std::find_if(impl_->cars_.begin(), impl_->cars_.end(), 
                             [=](const auto& entry)
      {
        return entry.car == car;
      });

      if (it != impl_->cars_.end())
      {
        impl_->engine_playback_controller_.stop_sound_playback(it->engine_sound);
        impl_->skid_playback_controller_.stop_sound_playback(it->skid_sound);
        impl_->cars_.erase(it);
      }
    }

    void CarSoundController::update(std::size_t frame_duration)
    {
      for (auto& car_info : impl_->cars_)
      {
        if (car_info.engine_sound)
        {
          auto pitch = static_cast<float>(0.25 + car_info.car->engine_rev_speed_ * 0.75);

          car_info.engine_sound.set_volume(pitch * pitch);          
          car_info.engine_sound.set_pitch(pitch);
        }

        if (car_info.car->traction_ < 0.8)
        {
          auto velocity = car_info.car->velocity();
          auto volume = static_cast<float>(std::min(magnitude(velocity) / (400.0 * car_info.car->traction_), 1.0));

          if (!car_info.skid_sound)
          {
            audio::PlaybackProperties properties;
            car_info.skid_sound = impl_->skid_playback_controller_.play_looped_sound_effect(*impl_->skid_sound_,
                                                                                            properties, 1);
          }

          if (car_info.skid_sound)
          {
            car_info.skid_sound.set_volume(volume * volume * 0.6f);
          }          
        }

        else if (car_info.skid_sound)
        {
          impl_->skid_playback_controller_.stop_sound_playback(car_info.skid_sound);
          car_info.skid_sound = {};
        }
      }
    }
  }
}