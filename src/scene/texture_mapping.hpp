/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TEXTURE_MAPPING_129001920
#define TEXTURE_MAPPING_129001920

#include "utility/rect.hpp"

#include <boost/range/iterator_range.hpp>

#include <vector>
#include <algorithm>

namespace ts
{
  namespace scene
  {
    template <typename TextureType>
    struct MappedTexture
    {
      std::size_t resource_id;
      IntRect texture_rect;
      Vector2i fragment_offset;
      TextureType texture;
    };

    template <typename TextureType>
    struct TextureMappingInterface;

    // The texture mapping class maps resource ids to a texture, meaning we can take a resource id and
    // look up its texture and corresponding rectangle with reasonable efficiency.
    template <typename TextureType>
    class TextureMapping
    {
    public:
      using mapped_texture = MappedTexture<TextureType>;
      using mapping_interface = TextureMappingInterface<TextureType>;
      using mapping_range = boost::iterator_range<const mapped_texture*>;

      // The following functions are provided for completeness, but they are extremely slow if 
      // used several times in a row. In such cases, it will be a great deal more efficient
      // to create a mapping interface and use the similar interface provided by that class.
      void map_texture(std::size_t resource_id, TextureType texture, IntRect texture_rect);
      void map_texture_fragment(std::size_t resource_id, TextureType texture, IntRect texture_rect, Vector2i fragment_offset);

      // Look up a texture by resource id in O(log n).
      mapping_range find(std::size_t resource_id, TextureType texture_hint) const;
      mapping_range find(std::size_t resource_id) const;

      static std::size_t tile_id(std::size_t tile);
      static std::size_t texture_id(std::size_t texture);

      // Find all matching textures, plus a bool to tell whether the range consists of fragments.
      std::pair<mapping_range, bool> find_all(std::size_t resource_id) const;

      // This function creates the mapping interface we mentioned above. Be aware that it's pointless
      // to use this class while a mapping face object exists.
      mapping_interface create_mapping_interface();

    private:
      friend mapping_interface;
      std::vector<mapped_texture> textures_;
      std::vector<mapped_texture> texture_fragments_;
    };

    // The texture mapping interface provides a way to define a large number of textures
    // and fragments in an optimized way. It does this by stealing the TextureMapping's internal
    // data and operating on them, keeping them unsorted until passing the data back in the destructor.
    template <typename TextureType>
    struct TextureMappingInterface
    {
    public:
      using mapped_texture = MappedTexture<TextureType>;
      ~TextureMappingInterface();

      void map_texture(std::size_t resource_id, TextureType texture, IntRect texture_rect);
      void map_texture_fragment(std::size_t resource_id, TextureType, IntRect texture_rect, Vector2i fragment_offset);

    private:
      friend TextureMapping<TextureType>;
      explicit TextureMappingInterface(TextureMapping<TextureType>* texture_mapping);

      std::vector<mapped_texture> textures_;
      std::vector<mapped_texture> texture_fragments_;
      TextureMapping<TextureType>* texture_mapping_;
    };

    namespace detail
    {
      struct MappedTextureComparator
      {
        template <typename TextureType>
        bool operator()(const MappedTexture<TextureType>& a, const MappedTexture<TextureType>& b) const
        {
          return a.resource_id < b.resource_id;
        }

        template <typename TextureType>
        bool operator()(const MappedTexture<TextureType>& a, std::size_t resource_id) const
        {
          return a.resource_id < resource_id;
        }

        template <typename TextureType>
        bool operator()(std::size_t resource_id, const MappedTexture<TextureType>& b) const
        {
          return resource_id < b.resource_id;
        }
      };
    }

    template <typename TextureType>
    void TextureMapping<TextureType>::map_texture(std::size_t resource_id, TextureType texture, IntRect texture_rect)
    {
      // Slow
      mapped_texture m;
      m.resource_id = resource_id;
      m.texture = texture;
      m.texture_rect = texture_rect;
      m.fragment_offset = {};
      auto insert_position = std::upper_bound(textures_.begin(), textures_.end(), m,
                                              detail::MappedTextureComparator());

      textures_.insert(insert_position, m);
    }

    template <typename TextureType>
    void TextureMapping<TextureType>::map_texture_fragment(std::size_t resource_id, TextureType texture,
                                                           IntRect texture_rect, Vector2i fragment_offset)
    {
      // And slow
      mapped_texture m;
      m.resource_id = resource_id;
      m.texture = texture;
      m.texture_rect = texture_rect;
      m.fragment_offset = fragment_offset;
      auto insert_position = std::upper_bound(texture_fragments_.begin(), texture_fragments_.end(), m,
                                              detail::MappedTextureComparator());

      texture_fragments_.insert(insert_position, m);
    }

    template <typename TextureType>
    typename TextureMapping<TextureType>::mapping_range TextureMapping<TextureType>::find(std::size_t resource_id) const
    {
      auto result = find_all(resource_id);
      if (result.second)
      {
        return result.first;
      }

      return mapping_range(result.first.begin(), result.first.begin() + 1);
    }

    template <typename TextureType>
    typename TextureMapping<TextureType>::mapping_range TextureMapping<TextureType>::find(std::size_t resource_id,
                                                                                          TextureType texture_hint) const
    {
      auto result = find_all(resource_id);
      mapping_range range = result.first;
      if (!result.second)
      {
        auto it = std::find_if(range.begin(), range.end(),
                               [=](const mapped_texture& m)
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

    template <typename TextureType>
    TextureMappingInterface<TextureType> TextureMapping<TextureType>::create_mapping_interface()
    {
      return TextureMappingInterface<TextureType>(this);
    }

    template <typename TextureType>
    std::size_t TextureMapping<TextureType>::tile_id(std::size_t resource_id)
    {
      return resource_id;
    }

    template <typename TextureType>
    std::size_t TextureMapping<TextureType>::texture_id(std::size_t resource_id)
    {
      return resource_id | 0x80000000;
    }


    template <typename TextureType>
    std::pair<typename TextureMapping<TextureType>::mapping_range, bool>
      TextureMapping<TextureType>::find_all(std::size_t resource_id) const
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


    template <typename TextureType>
    TextureMappingInterface<TextureType>::TextureMappingInterface(TextureMapping<TextureType>* texture_mapping)
      : texture_mapping_(texture_mapping),
        textures_(std::move(texture_mapping->textures_)),
        texture_fragments_(std::move(texture_mapping->texture_fragments_))
    {
    }

    template <typename TextureType>
    TextureMappingInterface<TextureType>::~TextureMappingInterface()
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

    template <typename TextureType>
    void TextureMappingInterface<TextureType>::map_texture(std::size_t resource_id, TextureType texture, IntRect texture_rect)
    {
      mapped_texture m;
      m.resource_id = resource_id;
      m.texture = texture;
      m.texture_rect = texture_rect;
      m.fragment_offset = {};
      
      textures_.push_back(m);
    }

    template <typename TextureType>
    void TextureMappingInterface<TextureType>::map_texture_fragment(std::size_t resource_id, TextureType texture,
                                                                    IntRect texture_rect, Vector2i fragment_offset)
    {
      mapped_texture m;
      m.resource_id = resource_id;
      m.texture = texture;
      m.texture_rect = texture_rect;
      m.fragment_offset = fragment_offset;

      texture_fragments_.push_back(m);
    }
  }
}

#endif
