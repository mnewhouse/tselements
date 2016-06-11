/** TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TEXTURE_LIBRARY_3D_HPP_2489835
#define TEXTURE_LIBRARY_3D_HPP_2489835

#include "utility/rect.hpp"

#include <boost/range/iterator_range.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace ts
{
  namespace resources_3d
  {
    struct TextureEntry
    {
      std::uint16_t id;
      IntRect image_rect;
      std::string image_file;
    };

    class TextureLibrary
    {
    public:
      const std::vector<TextureEntry>& textures() const;

      void define_texture(std::uint16_t id, std::string image_file, IntRect image_rect);
      const TextureEntry* find_texture(std::uint16_t id) const;

    private:   
      std::vector<TextureEntry> textures_;
    };
  }
}

#endif