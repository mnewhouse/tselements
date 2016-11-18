/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

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

      bool TextureComparator::operator()(const Texture& texture, TextureId id) const
      {
        return texture.id < id;
      }

      bool TextureComparator::operator()(TextureId id, const Texture& texture) const
      {
        return id < texture.id;
      }
    }

    TextureLibrary::TextureDefinitionInterface TextureLibrary::define_texture_set(const std::string& file_name)
    {
      auto string_it = filename_strings_.insert(file_name).first;
      boost::string_ref filename_ref(string_it->data(), string_it->size());

      return TextureDefinitionInterface(this, filename_ref);
    }

    TextureLibrary::texture_iterator TextureLibrary::define_texture(const Texture& texture)
    {
      auto result = textures_.insert(texture);
      if (result.second)
      {
        DEBUG_AUXILIARY << "Info: added texture definition. [texture_id=" << texture.id << "]" << debug::endl;
      }

      else
      {
        DEBUG_AUXILIARY << "Warning: texture definition already exists, not overwriting. [texture_id=" <<
          texture.id << "]" << debug::endl;
      }

      return textures_.insert(texture).first;
    }

    TextureLibrary::texture_interface_type TextureLibrary::textures() const
    {
      return texture_interface_type(&textures_);
    }

    TextureLibrary::TextureDefinitionInterface::TextureDefinitionInterface(TextureLibrary* texture_library,
                                                                           boost::string_ref image_file)
      : texture_library_(texture_library),
      image_file_(image_file)
    {
    }

    TextureLibrary::texture_iterator TextureLibrary::TextureDefinitionInterface::define_texture(TextureId texture_id,
                                                                                                IntRect texture_rect)
    {
      Texture texture;
      texture.id = texture_id;
      texture.file_name = image_file_;
      texture.rect = texture_rect;

      return texture_library_->define_texture(texture);      
    }
  }
}