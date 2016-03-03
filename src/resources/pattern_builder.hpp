/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RESOURCES_PATTERN_BUILDER_HPP_2289189125
#define RESOURCES_PATTERN_BUILDER_HPP_2289189125

#include "pattern.hpp"
#include "track.hpp"
#include "pattern_loader.hpp"

namespace ts
{
  namespace resources
  {
    class Track;

    class PatternBuilder
    {
    public:
      explicit PatternBuilder(const Track& track, PatternLoader pattern_loader = {});

      Pattern build();

      void preload_pattern(const std::string& pattern_path);

    private:
      const Track& track_;
      PatternLoader pattern_loader_;
    };

    void preload_pattern_files(PatternLoader& pattern_loader, const resources::Track& track);
  }
}


#endif