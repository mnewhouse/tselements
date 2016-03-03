/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "pattern_loader.hpp"
#include "pattern.hpp"

namespace ts
{
  namespace resources
  {
    Pattern& PatternLoader::load_from_file(boost::string_ref file_name)
    {
      auto it = loaded_patterns_.find(file_name);
      if (it != loaded_patterns_.end()) 
      {
        return it->second;
      }

      auto pattern = load_pattern({ file_name.data(), file_name.size() });
      auto result = loaded_patterns_.insert(std::make_pair(file_name, std::move(pattern)));

      return result.first->second;
    }
  }
}