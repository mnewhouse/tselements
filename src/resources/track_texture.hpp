/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/rect.hpp"

#include "boost/utility/string_ref.hpp"

#include <cstdint>

namespace ts
{
  namespace resources
  {
    using TextureId = std::uint16_t;
    static constexpr TextureId max_texture_id = 2048;

    struct Texture
    {
      TextureId id;
      boost::string_ref file_name;
      IntRect rect;
      // TODO terrain_id
    };
  }
}
