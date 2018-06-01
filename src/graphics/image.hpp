/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <SFML/Graphics/Image.hpp>

#include <istream>

namespace ts
{
  namespace graphics
  {
    sf::Image load_image(const std::string& file_name);
    sf::Image load_image(const char* file_name, std::size_t file_name_length);
    sf::Image load_image(std::istream& stream);

    void save_image(const sf::Image& image, const std::string& file_name);

    struct LoadImage
    {
      template <typename String>
      sf::Image operator()(const String& file_name) const
      {
        return (*this)(file_name.data(), file_name.size());
      }

      sf::Image operator()(const char* file_name, std::size_t file_name_length) const;
    };
  }
}
