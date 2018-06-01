/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "texture_library.hpp"

#include "utility/debug_log.hpp"

namespace ts
{
  namespace resources
  {
    namespace detail
    {
      bool TextureComparator::operator()(const Texture& a, const Texture& b) const
      {
        return a.id < b.id;
      }

      bool TextureComparator::operator()(const Texture& texture, std::uint32_t id) const
      {
        return texture.id < id;
      }

      bool TextureComparator::operator()(std::uint32_t id, const Texture& texture) const
      {
        return id < texture.id;
      }
    }

    TextureLibrary::texture_iterator TextureLibrary::define_texture(const Texture& texture)
    {
      auto result = textures_.insert(texture);
      return textures_.insert(texture).first;
    }

    TextureLibrary::texture_interface_type TextureLibrary::textures() const
    {
      return texture_interface_type(&textures_);
    }
  }
}