/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "car_hash.hpp"
#include "collision_mask.hpp"
#include "handling.hpp"

#include "utility/rect.hpp"

#include <boost/container/small_vector.hpp>

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

    // The CarDefinition structure defines all properties that make up a car model,
    // such as name, image, pattern, engine sound, and handling.
    struct CarDefinition
    {
      std::string car_name;
      CarHash car_hash;

      std::string image_path;
      CarImage image_type = CarImage::Default;
      IntRect image_rect;
      std::uint32_t num_rotations = 1;
      double image_scale = 2.0;

      boost::container::small_vector<Vector2i, 4> tyre_positions;

      std::shared_ptr<CollisionMask> collision_mask;
      double bounciness = 1.0;
      
      Handling handling;

      std::string engine_sound_path;
    };
  }
}
