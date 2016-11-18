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
    struct TrackReference
    {
      std::string path;
      std::string name;
      TrackHash hash;
    };

    struct TrackReferenceView
    {
      TrackReferenceView(const TrackReference& ref)
        : path(ref.path),
          name(ref.name),
          hash(ref.hash)
      {
      }

      TrackReferenceView() = default;

      explicit operator TrackReference() const
      {
        TrackReference result;
        result.path.assign(path.begin(), path.end());
        result.name.assign(name.begin(), name.end());
        result.hash = hash;
        return result;
      }

      boost::string_ref path;
      boost::string_ref name;
      TrackHash hash;
    };
  }
}
