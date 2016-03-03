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