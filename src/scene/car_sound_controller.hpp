/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef ENGINE_SOUND_CONTROLLER_HPP_8548921891283
#define ENGINE_SOUND_CONTROLLER_HPP_8548921891283

#include <memory>

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
    class CarSoundController
    {
    public:
      CarSoundController();
      ~CarSoundController();

      CarSoundController(CarSoundController&&);
      CarSoundController& operator=(CarSoundController&&);

      std::size_t register_car_model(const resources::CarDefinition& car_def);

      void register_car(std::size_t model_id, const world::Car* car);
      void unregister_car(const world::Car* car);

      void update(std::size_t frame_duration);

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}

#endif