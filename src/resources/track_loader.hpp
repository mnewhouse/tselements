/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TRACK_LOADER_HPP_758753
#define TRACK_LOADER_HPP_758753

#include "track.hpp"

#include <string>
#include <unordered_set>
#include <stdexcept>

#include <boost/utility/string_ref.hpp>

namespace ts
{
  namespace resources
  {
    struct TrackLayer;

    struct BrokenTrackException
      : std::runtime_error
    {
      explicit BrokenTrackException(const std::string& missing_file);
    };

    class TrackLoader
    {
    public:
      void load_from_file(const std::string& path);
      void include(const std::string& path);

      Track get_result();
      
      struct Context;

    private:
      
      void include(const std::string& path, std::size_t inclusion_depth);
      void load_included_file(Context& context, std::size_t inclusion_depth);

      std::unordered_set<std::string> included_files_;
      std::string working_directory_;

      TrackLayer* current_layer_;
      Track track_;
    };

    Track load_track(const std::string& path);
    Track load_track(boost::string_ref path);
  }
}

#endif