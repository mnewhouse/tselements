/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "pattern_store.hpp"
#include "pattern.hpp"

namespace ts
{
  namespace resources
  {
    Pattern& PatternStore::load_from_file(const std::string& file_name)
    {
      auto it = loaded_patterns_.find(file_name);
      if (it != loaded_patterns_.end()) 
      {
        return it->second;
      }

      auto pattern = load_pattern(file_name);
      auto result = loaded_patterns_.insert(std::make_pair(file_name, std::move(pattern)));

      return result.first->second;
    }
  }
}