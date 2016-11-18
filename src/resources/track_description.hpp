/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "track_hash.hpp"

#include <boost/utility/string_ref.hpp>

#include <string>

namespace ts
{
  namespace resources
  {
    // The TrackDescription class provides a unique identifier for a track.
    // It's just a track name and a track hash, nothing fancy there.
    struct TrackDescription
    {
      std::string name;
      TrackHash hash;
    };

    struct TrackDescriptionView
    {
      TrackDescriptionView(const TrackDescription& track_desc)
        : name(track_desc.name),
          hash(track_desc.hash)
      {}

      TrackDescriptionView() = default;

      boost::string_ref name;
      TrackHash hash;
    };

    inline bool operator==(const TrackDescription& a, const TrackDescription& b)
    {
      return a.name == b.name && a.hash == b.hash;
    }

    inline bool operator!=(const TrackDescription& a, const TrackDescription& b)
    {
      return !(a == b);
    }
  }
}

namespace std
{
  template <>
  struct hash<ts::resources::TrackDescription>
  {
    std::size_t operator()(const ts::resources::TrackDescription& track_desc) const
    {
      return std::hash<std::string>()(track_desc.name) ^
        (track_desc.hash[1] << 30) ^ (track_desc.hash[2] << 20) ^ (track_desc.hash[3] << 10) | track_desc.hash[0];
    }
  };
}
