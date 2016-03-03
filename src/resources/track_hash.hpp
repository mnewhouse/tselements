/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_HASH_HPP_556920
#define TRACK_HASH_HPP_556920

#include <array>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    using TrackHash = std::array<std::uint32_t, 4>;
  }
}

#endif