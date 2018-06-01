/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <algorithm>
#include <cctype>
#include <iterator>

#include <boost/utility/string_ref.hpp>
#include <boost/functional/hash.hpp>

#include "caseless_string_compare.hpp"

namespace ts
{
  // This function template splits a block of memory by newlines. 
  // OutputIt must be dereferencable and then assignable from a brace-init-list
  // containing a const char* and a std::size_t.
  // For example, a (back_)insert_iterator for a container holding strings or string_refs.
  template <typename OutputIt>
  std::size_t split_by_line(const char* data, std::size_t data_size, OutputIt out)
  {
    std::size_t count = 0;
    for (auto end = data + data_size; data != end; ++count)
    {
      const char* line_end = std::find(data, end, '\n');
      std::size_t line_size = std::distance(data, line_end);

      *out = { data, line_size }; ++out;

      data = std::find_if(line_end, end, [](char ch) { return ch != '\n'; });
    }

    return count;
  }

  inline boost::string_ref make_string_ref(const char* data, const char* end)
  {
    return boost::string_ref(data, std::distance(data, end));
  }

  inline boost::string_ref remove_leading_spaces(boost::string_ref string, const char* spaces = " \t\r\n")
  {
    auto pos = string.find_first_not_of(spaces);
    if (pos == std::string::npos) pos = string.size();

    string.remove_prefix(pos);
    return string;
  }

  inline boost::string_ref extract_word(boost::string_ref string)
  {
    const char spaces[] = " \t\r\n";
    string = remove_leading_spaces(string, spaces);

    auto new_size = string.find_first_of(spaces);
    if (new_size == boost::string_ref::npos) new_size = string.size();

    return boost::string_ref(string.data(), new_size);
  }


  template <typename String>
  inline void primitive_tolower(String& string)
  {
    using std::begin;
    using std::end;

    std::transform(begin(string), end(string), begin(string), [](auto ch) { return std::tolower(ch); });
  }

  struct StringRefHasher
  {
    auto operator()(boost::string_ref string) const
    {
      return boost::hash_range(string.begin(), string.end());
    }
  };
}
