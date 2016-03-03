/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "texture.hpp"

#include <SFML/Graphics/Image.hpp>

namespace ts
{
  namespace graphics
  {
    Texture create_texture_from_image(const sf::Image& image)
    {
      GLuint name{};
      glGenTextures(1, &name);
      Texture texture(name);

      auto image_size = image.getSize();

      glBindTexture(GL_TEXTURE_2D, name);
      glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA, image_size.x, image_size.y);

      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_size.x, image_size.y,
                      GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());

      return texture;
    }

    GLint max_texture_size()
    {
      GLint result = 0;
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);

      return result;
    }
  }
}