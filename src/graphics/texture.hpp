/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TEXTURE_HPP_58391285
#define TEXTURE_HPP_58391285

#include "utility/vector2.hpp"
#include "utility/rect.hpp"

#include <cstddef>
#include <memory>

#include <SFML/Graphics/Image.hpp>

#include <GL/glew.h>

namespace ts
{
  namespace graphics
  {
    namespace detail
    {
      struct TextureDeleter
      {
        using pointer = GLuint;
        void operator()(pointer p) const
        {
          if (p) glDeleteTextures(1, &p);
        }
      };
    }
    
    using Texture = std::unique_ptr<GLuint, detail::TextureDeleter>;
    class Surface;

    void allocate_texture_storage(const Texture& texture, Vector2i size);
    void update_texture_image(const Texture& texture, Vector2i pos,
                              const sf::Image& image, IntRect source_rect);

    Texture create_texture_from_image(const sf::Image& image);
    GLint max_texture_size();
  }
}

#endif