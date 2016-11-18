/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <boost/utility/string_ref.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>

#include <string>
#include <istream>
#include <fstream>
#include <vector>

namespace ts
{
  std::ifstream make_ifstream(const std::string& file_name, std::ios::openmode open_mode = std::ios::in | std::ios::binary);
  std::ofstream make_ofstream(const std::string& file_name, std::ios::openmode open_mode = std::ios::out | std::ios::binary);

  std::vector<char> load_file_contents(const std::string& file_name);

  template <typename CharType>
  std::vector<CharType> read_stream_contents(std::basic_istream<CharType>& stream)
  {
    auto pos = stream.tellg();
    stream.seekg(0, std::ios::end);
    auto diff = stream.tellg() - pos;

    std::vector<CharType> result;
    if (diff <= 0) return result;

    stream.seekg(0);
    result.resize(static_cast<std::size_t>(diff));
    stream.read(result.data(), diff);

    result.resize(static_cast<std::size_t>(stream.gcount()));
    return result;
  }

  template <typename CharType>
  std::vector<CharType> read_stream_contents(std::basic_istream<CharType>&& stream)
  {
    return read_stream_contents(stream);
  }

  struct ArrayStream
  {
    explicit ArrayStream(boost::string_ref string)
      : source_(string.begin(), string.end()),
        stream_(source_)
    {}

    explicit operator bool() const
    {
      return static_cast<bool>(stream_);
    }

    template <typename T>
    ArrayStream& operator>>(T& value)
    {
      stream_ >> value;
      return *this;
    }

  private:
    using source_type = boost::iostreams::array_source;
    source_type source_;
    
    using stream_type = boost::iostreams::stream<source_type>;
    stream_type stream_;
  };
}
