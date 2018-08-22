/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <string>

namespace ts
{
  namespace resources
  {
    class Track;
    bool save_track(const Track& track);
    bool save_track(const Track& track, const std::string& dest_path);
  }
}