/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_LOADER_3D_HPP_8491283894
#define TRACK_LOADER_3D_HPP_8491283894

#include <string>

#include "track_3d.hpp"

namespace ts
{
  namespace resources_3d
  {
    class Track;

    class TrackLoader
    {
    public:
      void include(const std::string& file_name);

      Track get_result();

    private:
      void include(const std::string& file_name, std::size_t inclusion_depth);

      Track track_;
      std::string working_directory_;
    };
  }
}

#endif