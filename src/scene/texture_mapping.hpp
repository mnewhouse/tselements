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

#include "graphics/texture.hpp"

namespace ts
{
  namespace scene
  {
    struct MappedTexture
    {
      using texture_type = const graphics::Texture*;

      std::size_t resource_id;
      IntRect texture_rect;
      Vector2i fragment_offset;
      texture_type texture;
    };

    struct TextureMappingInterface;

    // The texture mapping class maps resource ids to a texture, meaning we can take a resource id and
    // look up its texture and corresponding rectangle with reasonable efficiency.
    class TextureMapping
    {
    public:
      using mapping_range = boost::iterator_range<const MappedTexture*>;
      using texture_type = MappedTexture::texture_type;

      // The following functions are provided for completeness, but they are extremely slow if 
      // used several times in a row. In such cases, it will be a great deal more efficient
      // to create a mapping interface and use the similar interface provided by that class.
      void map_texture(std::size_t resource_id, texture_type texture, IntRect texture_rect);
      void map_texture_fragment(std::size_t resource_id, texture_type texture, IntRect texture_rect, Vector2i fragment_offset);

      // Look up a texture by resource id in O(log n).
      mapping_range find(std::size_t resource_id, texture_type texture_hint) const;
      mapping_range find(std::size_t resource_id) const;

      static std::size_t tile_id(std::size_t tile);
      static std::size_t texture_id(std::size_t texture);

      // Find all matching textures, plus a bool to tell whether the range consists of fragments.
      std::pair<mapping_range, bool> find_all(std::size_t resource_id) const;

      // This function creates the mapping interface we mentioned above. Be aware that it's pointless
      // to use this class while a mapping face object exists.
      TextureMappingInterface create_mapping_interface();

    private:
      friend TextureMappingInterface;
      std::vector<MappedTexture> textures_;
      std::vector<MappedTexture> texture_fragments_;
    };

    // The texture mapping interface provides a way to define a large number of textures
    // and fragments in an optimized way. It does this by stealing the TextureMapping's internal
    // data and operating on them, keeping them unsorted until passing the data back in the destructor.
    struct TextureMappingInterface
    {
    public:
      using texture_type = MappedTexture::texture_type;

      ~TextureMappingInterface();

      void map_texture(std::size_t resource_id, texture_type texture, IntRect texture_rect);
      void map_texture_fragment(std::size_t resource_id, texture_type texture, 
                                IntRect texture_rect, Vector2i fragment_offset);

    private:
      friend TextureMapping;
      explicit TextureMappingInterface(TextureMapping* texture_mapping);

      std::vector<MappedTexture> textures_;
      std::vector<MappedTexture> texture_fragments_;
      TextureMapping* texture_mapping_;
    };

    namespace detail
    {
      struct MappedTextureComparator
      {
        bool operator()(const MappedTexture& a, const MappedTexture& b) const
        {
          return a.resource_id < b.resource_id;
        }
        bool operator()(const MappedTexture& a, std::size_t resource_id) const
        {
          return a.resource_id < resource_id;
        }

        bool operator()(std::size_t resource_id, const MappedTexture& b) const
        {
          return resource_id < b.resource_id;
        }
      };
    }
  }
}

#endif
