/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "texture_mapping.hpp"

namespace ts
{
  namespace scene
  {
    TextureMapping::TextureMapping(std::vector<std::unique_ptr<graphics::Texture>> textures)
      : texture_storage_(std::move(textures))
    {
    }

    void TextureMapping::map_texture(std::size_t resource_id, texture_type texture, IntRect texture_rect)
    {
      // Slow
      MappedTexture m;
      m.resource_id = resource_id;
      m.texture = texture;
      m.texture_rect = texture_rect;
      m.fragment_offset = {};
      auto insert_position = std::upper_bound(textures_.begin(), textures_.end(), m,
                                              detail::MappedTextureComparator());

      textures_.insert(insert_position, m);
    }

    void TextureMapping::map_texture_fragment(std::size_t resource_id, texture_type texture,
                                              IntRect texture_rect, Vector2i fragment_offset)
    {
      // And slow
      MappedTexture m;
      m.resource_id = resource_id;
      m.texture = texture;
      m.texture_rect = texture_rect;
      m.fragment_offset = fragment_offset;
      auto insert_position = std::upper_bound(texture_fragments_.begin(), texture_fragments_.end(), m,
                                              detail::MappedTextureComparator());

      texture_fragments_.insert(insert_position, m);
    }

    TextureMapping::mapping_range TextureMapping::find(std::size_t resource_id) const
    {
      return find(resource_id, nullptr);
    }

    TextureMapping::mapping_range TextureMapping::find(std::size_t resource_id,
                                                       texture_type texture_hint) const
    {
      auto result = find_all(resource_id);
      mapping_range range = result.first;
      if (!result.second)
      {
        auto it = std::find_if(range.begin(), range.end(),
                               [=](const MappedTexture& m)
        {
          return m.texture == texture_hint;
        });

        // If we found a texture that matches the texture hint, return it.
        if (it != range.end())
        {
          range = { it, std::next(it) };
        }

        // Otherwise, return the first element in the range, as a range.
        else if (!range.empty())
        {
          range = { range.begin(), std::next(range.begin()) };
        }
      }

      return range;
    }

    const std::vector<std::unique_ptr<graphics::Texture>>& TextureMapping::textures() const
    {
      return texture_storage_;
    }

    TextureMappingInterface TextureMapping::create_mapping_interface()
    {
      return TextureMappingInterface(this);
    }

    std::size_t TextureMapping::tile_id(std::size_t resource_id)
    {
      return resource_id;
    }

    std::size_t TextureMapping::texture_id(std::size_t resource_id)
    {
      return resource_id | 0x80000000;
    }


    std::pair<TextureMapping::mapping_range, bool>
      TextureMapping::find_all(std::size_t resource_id) const
    {
      bool fragments = false;
      auto range = std::equal_range(textures_.data(), textures_.data() + textures_.size(),
                                    resource_id, detail::MappedTextureComparator());

      if (range.first == range.second)
      {
        // If there are no textures with the given id, switch over to looking for texture fragments.
        range = std::equal_range(texture_fragments_.data(), texture_fragments_.data() + texture_fragments_.size(),
                                 resource_id, detail::MappedTextureComparator());
        if (range.first != range.second)
        {
          fragments = true;
        }
      }

      return std::make_pair(mapping_range(range.first, range.second), fragments);
    }


    TextureMappingInterface::TextureMappingInterface(TextureMapping* texture_mapping)
      : texture_mapping_(texture_mapping),
        textures_(std::move(texture_mapping->textures_)),
        texture_fragments_(std::move(texture_mapping->texture_fragments_))
    {
    }

    TextureMappingInterface::~TextureMappingInterface()
    {
      detail::MappedTextureComparator cmp;

      // Sort the data that we assembled...
      // Using stable_sort to preserve the order is potentially important,
      // because the first matching item must be the one that was first added.
      std::stable_sort(textures_.begin(), textures_.end(), cmp);
      std::stable_sort(texture_fragments_.begin(), texture_fragments_.end(), cmp);

      // And give it back to where it originally came from.
      texture_mapping_->textures_ = std::move(textures_);
      texture_mapping_->texture_fragments_ = std::move(texture_fragments_);
    }

    void TextureMappingInterface::map_texture(std::size_t resource_id, texture_type texture, IntRect texture_rect)
    {
      MappedTexture m;
      m.resource_id = resource_id;
      m.texture = texture;
      m.texture_rect = texture_rect;
      m.fragment_offset = {};

      textures_.push_back(m);
    }

    void TextureMappingInterface::map_texture_fragment(std::size_t resource_id, texture_type texture,
                                                       IntRect texture_rect, Vector2i fragment_offset)
    {
      MappedTexture m;
      m.resource_id = resource_id;
      m.texture = texture;
      m.texture_rect = texture_rect;
      m.fragment_offset = fragment_offset;

      texture_fragments_.push_back(m);
    }
  }
}