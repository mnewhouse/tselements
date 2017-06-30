/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "image.hpp"

#include "utility/stream_utilities.hpp"

#include <memory>

namespace ts
{
  namespace graphics
  {
    namespace detail
    {
      std::runtime_error image_error(const std::string& file_name)
      {
        return std::runtime_error("Failed to load image from '" + file_name + "'");
      }
    }

    sf::Image load_image(std::istream& stream)
    {
      auto contents = read_stream_contents(stream);

      sf::Image image;
      if (!image.loadFromMemory(contents.data(), contents.size()))
      {
        throw detail::image_error("stream");
      }

      return image;
    }

    sf::Image load_image(const std::string& file_name)
    {
      return load_image(file_name.data(), file_name.size());
    }

    sf::Image load_image(const char* file_name, std::size_t file_name_length)
    {
      std::string file_name_string(file_name, file_name_length);
      auto stream = make_ifstream(file_name_string);
      if (!stream) throw detail::image_error(file_name_string);

      return load_image(stream);
    }

    void save_image(const sf::Image& image, const std::string& file_name)
    {
      image.saveToFile(file_name);
    }

    sf::Image LoadImage::operator()(const char* file_name, std::size_t file_name_length) const
    {
      return load_image(file_name, file_name_length);
    }
  }
}