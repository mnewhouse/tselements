/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resource_library_interface.hpp"

#include "utility/rect.hpp"

#include <cstdint>
#include <string>
#include <set>
#include <type_traits>

namespace ts
{
  namespace resources
  {
    static constexpr std::uint32_t max_texture_id = 2048;

    struct Texture
    {
      std::uint32_t id;
      std::string file_name;
    };

    namespace detail
    {
      struct TextureComparator
      {
        using is_transparent = std::true_type;
        bool operator()(const Texture& a, const Texture& b) const;
        bool operator()(const Texture& a, std::uint32_t id) const;
        bool operator()(std::uint32_t id, const Texture& texture) const;
      };
    }

    // The TextureLibrary stores a track's available textures.
    // These are primarily used to draw non-tile-based geometry, mostly terrains.
    class TextureLibrary
    {
    public:
      using texture_container_type = std::set<Texture, detail::TextureComparator>;
      using texture_interface_type = ResourceLibraryInterface<texture_container_type, SetSearchPolicy>;
      using texture_iterator = texture_container_type::iterator;

      texture_interface_type textures() const;

      texture_iterator define_texture(const Texture& texture);

    private:
      std::set<Texture, detail::TextureComparator> textures_;
    };
  }
}
