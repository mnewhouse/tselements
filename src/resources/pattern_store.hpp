/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "pattern.hpp"

#include <string>
#include <map>

namespace ts
{
  namespace resources
  {
    // A simple pattern loader class that takes caches pattern files based
    // on their file names. It uses the load_pattern() function, and as such will throw
    // an exception on failure.
    class PatternStore
    {
    public:
      // Make it non-copyable, to ensure reference stability with the map entries when it's passed around.
      PatternStore(const PatternStore&) = delete;
      PatternStore(PatternStore&&) = default;
      PatternStore() = default;

      PatternStore& operator=(const PatternStore&) = delete;
      PatternStore& operator=(PatternStore&&) = default;

      template <typename StringType>
      Pattern& load_from_file(const StringType& file_name);

      Pattern& load_from_file(const std::string& file_name);

    private:
      std::map<std::string, Pattern, std::less<>> loaded_patterns_;
    };

    template <typename StringType>
    Pattern& PatternStore::load_from_file(const StringType& file_name)
    {
      auto it = loaded_patterns_.find(file_name);
      if (it == loaded_patterns_.end())
      {
        using std::begin;
        using std::end;
        return load_from_file(std::string(begin(file_name), end(file_name)));
      }

      return it->second;
    }
  }
}
