/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "stream_utilities.hpp"
#include "encoding.hpp"

namespace ts
{
  std::ifstream make_ifstream(const std::string& file_name, std::ios::openmode open_mode)
  {
    return std::ifstream(convert_to_platform_encoding(file_name), open_mode);
  }

  std::ofstream make_ofstream(const std::string& file_name, std::ios::openmode open_mode)
  {
    return std::ofstream(convert_to_platform_encoding(file_name), open_mode);
  }

  std::vector<char> load_file_contents(const std::string& file_name)
  {
    return read_stream_contents(make_ifstream(file_name, std::ios::binary | std::ios::in));
  }
}