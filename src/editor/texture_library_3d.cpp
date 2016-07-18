/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "texture_library_3d.hpp"

#include <algorithm>

namespace ts
{
  namespace resources_3d
  {
    const std::vector<TextureEntry>& TextureLibrary::textures() const
    {
      return textures_;
    }

    void TextureLibrary::define_texture(std::uint16_t id, std::string image_file, IntRect rect)
    {
      auto insert_position = std::lower_bound(textures_.begin(), textures_.end(), id,
                                              [](const auto& entry, auto id)
      {
        return entry.id < id;
      });

      TextureEntry entry;
      entry.id = id;
      entry.image_rect = rect;
      entry.image_file = std::move(image_file);

      if (insert_position != textures_.end() && insert_position->id == id)
      {
        *insert_position = std::move(entry);
      }

      else
      {
        textures_.insert(insert_position, entry);
      }
    }
    const TextureEntry* TextureLibrary::find_texture(std::uint16_t id) const
    {
      auto it = std::lower_bound(textures_.begin(), textures_.end(), id,
                                 [](const auto& entry, auto id)
      {
        return entry.id < id;
      });

      if (it == textures_.end()) return nullptr;

      return &*it;
    }
  }
}