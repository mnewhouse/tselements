/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "audio/sound_sample.hpp"
#include "audio/sound_sample_loader.hpp"
#include "audio/sound_playback_controller.hpp"

#include <memory>
#include <vector>

namespace ts
{
  namespace resources
  {
    struct CarDefinition;
  }

  namespace world
  {
    class Car;
  }

  namespace scene
  {
    // The Car sound controller is responsible for playing the engine and skid sounds.
    // It has a list of car models, each of which has its own engine sound, and any number 
    // of car instances, which can make use of any of the models that were previously registered.
    class CarSoundController
    {
    public:
      CarSoundController();

      std::size_t register_car_model(const resources::CarDefinition& car_def);

      void register_car(std::size_t model_id, const world::Car* car);
      void unregister_car(const world::Car* car);

      void update(std::uint32_t frame_duration);

    private:
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

      std::vector<Car> cars_;
      std::vector<CarModel> car_models_;
      audio::SoundSampleLoader sound_bank_;
      audio::PlaybackController engine_playback_controller_;
      audio::PlaybackController skid_playback_controller_;

      const audio::SoundSample* skid_sound_;
    };
  }
}
