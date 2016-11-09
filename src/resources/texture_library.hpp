/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TEXTURE_LIBRARY_HPP_182994
#define TEXTURE_LIBRARY_HPP_182994

#include "track_texture.hpp"
#include "resource_library_interface.hpp"

#include "utility/rect.hpp"

#include <boost/utility/string_ref.hpp>

#include <cstdint>
#include <string>
#include <set>
#include <unordered_set>
#include <type_traits>

namespace ts
{
  namespace resources
  {
    namespace detail
    {
      struct TextureComparator
      {
        using is_transparent = std::true_type;
        bool operator()(const Texture& a, const Texture& b) const;
        bool operator()(const Texture& a, TextureId id) const;
        bool operator()(TextureId id, const Texture& texture) const;
      };
    }    

    // The TextureLibrary keeps track of the textures available in a certain track.
    // These are primarily used to draw non-tile-based graphical components, such as 
    // vertices.
    class TextureLibrary
    {
    public:
      using texture_container_type = std::set<Texture, detail::TextureComparator>;
      using texture_interface_type = ResourceLibraryInterface<texture_container_type, SetSearchPolicy>;
      using texture_iterator = texture_container_type::iterator;

      texture_interface_type textures() const;

      struct TextureDefinitionInterface;
      TextureDefinitionInterface define_texture_set(const std::string& file_name);      

    private:
      friend TextureDefinitionInterface;
      texture_iterator define_texture(const Texture& texture);

      std::set<Texture, detail::TextureComparator> textures_;

      std::unordered_set<std::string> filename_strings_;
    };

    struct TextureLibrary::TextureDefinitionInterface
    {
      texture_iterator define_texture(TextureId texture_id, IntRect texture_rect);

    private:
      friend TextureLibrary;
      TextureDefinitionInterface(TextureLibrary* texture_library, boost::string_ref image_file);

      TextureLibrary* texture_library_;
      boost::string_ref image_file_;
    };
  }

}

#endif