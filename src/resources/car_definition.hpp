/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CAR_DEFINITION_HPP_9102941029
#define CAR_DEFINITION_HPP_9102941029

#include "car_hash.hpp"
#include "collision_mask.hpp"

#include "utility/rect.hpp"

#include <string>
#include <cstdint>
#include <memory>

namespace ts
{
  namespace resources
  {
    enum class CarImage
    {
      Prerotated,
      Default,
    };

    struct CarDefinition
    {
      std::string car_name;

      std::shared_ptr<CollisionMask> collision_mask;
      double bounciness = 1.0;

      std::string image_path;
      CarImage image_type = CarImage::Default;
      IntRect image_rect;
      std::uint32_t num_rotations = 1;
      double image_scale = 2.0;

      std::string engine_sound_path;

      CarHash car_hash;
    };
  }
}

#endif