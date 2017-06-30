/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "texture_library_3d.hpp"

namespace ts
{
  namespace resources3d
  {
    std::size_t TextureLibrary::define_texture(TextureDescriptor tex_desc)
    {
      auto internal_id = texture_list_.size();
      tex_desc.internal_id = internal_id;
      texture_map_[tex_desc.image_file].push_back(internal_id);
      texture_list_.push_back(std::move(tex_desc));
      return internal_id;
    }

    const std::vector<TextureDescriptor>& TextureLibrary::textures() const
    {
      return texture_list_;
    }
  }
}