/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RESOURCES_PATTERN_LOADER_HPP_48491898123
#define RESOURCES_PATTERN_LOADER_HPP_48491898123

#include "pattern.hpp"

#include "boost/utility/string_ref.hpp"

#include <map>

namespace ts
{
  namespace resources
  {
    class Pattern;

    // A simple pattern loader class that takes caches pattern files based
    // on their file names. It uses the load_pattern() function, and as such will throw
    // an exception on failure.
    class PatternLoader
    {
    public:
      Pattern& load_from_file(boost::string_ref file_name);

    private:
      std::map<boost::string_ref, Pattern> loaded_patterns_;
    };
  }
}

#endif