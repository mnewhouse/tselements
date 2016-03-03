/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RESOURCES_PATTERN_HPP_218981925
#define RESOURCES_PATTERN_HPP_218981925

#include "utility/vector2.hpp"
#include "utility/rect.hpp"

namespace ts
{
  namespace resources
  {
    class Pattern
    {
    public:
      explicit Pattern(Vector2u size = {});

      const std::uint8_t& operator()(std::uint32_t x, std::uint32_t y) const;
      std::uint8_t& operator()(std::uint32_t x, std::uint32_t y);

      Vector2u size() const;
      void resize(Vector2u new_size);
      void resize(std::uint32_t x, std::uint32_t y);

      using iterator = std::uint8_t*;
      iterator begin();
      iterator end();
      iterator row_begin(std::uint32_t row_id);
      iterator row_end(std::uint32_t row_id);

      using const_iterator = const std::uint8_t*;
      const_iterator begin() const;
      const_iterator end() const;
      const_iterator row_begin(std::uint32_t row_id) const;
      const_iterator row_end(std::uint32_t row_id) const;

    private:
      Vector2u size_;
      std::vector<std::uint8_t> bytes_;
    };

    class TerrainLibrary;

    Pattern load_pattern(const std::string& file_name, IntRect rect = {});
    void save_pattern(const Pattern& pattern, const std::string& file_name, const TerrainLibrary& terrain_library);

    Pattern copy_pattern(const Pattern& pattern, IntRect source_rect);
  }
}


#endif