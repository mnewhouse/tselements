/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "feature_set.hpp"

#include "utility/rect.hpp"

#include <unordered_map>
#include <vector>

namespace ts
{
  namespace resources3d
  {
    struct TextureDescriptor
    {
      FeatureSetId feature_set;
      std::size_t user_id;
      std::size_t internal_id;
      std::string image_file;
      IntRect image_rect;
    };

    class TextureLibrary
    {
    public:
      std::size_t define_texture(TextureDescriptor texture_desc);

      using texture_map = std::unordered_map<std::string, std::vector<std::size_t>>;
      using texture_array = std::vector<TextureDescriptor>;
      
      const texture_array& textures() const;

    private:
      texture_map texture_map_;
      texture_array texture_list_;
    };
  }
}