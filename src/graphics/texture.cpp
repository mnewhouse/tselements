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
    Texture::Texture(const sf::Image& image)
      : texture_size_({ image.getSize().x, image.getSize().y })
    {
      GLuint name{};
      glGenTextures(1, &name);
      texture_.reset(name);

      glBindTexture(GL_TEXTURE_2D, name);
      glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture_size_.x, texture_size_.y);

      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_size_.x, texture_size_.y,
                      GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
      glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::Texture(Vector2u texture_size)
    {
      GLuint name{};
      glGenTextures(1, &name);
      texture_.reset(name);

      glBindTexture(GL_TEXTURE_2D, name);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_size.x, texture_size.y, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    Texture::Texture(GLuint texture, Vector2u size)
      : texture_(texture),
        texture_size_(size)
    {
    }

    void Texture::update(Vector2i pos, const sf::Image& image)
    {
      glBindTexture(GL_TEXTURE_2D, texture_.get());

      glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y, image.getSize().x, image.getSize().y, GL_RGBA,
                      GL_UNSIGNED_BYTE, image.getPixelsPtr());
    }

    void Texture::update(Vector2i pos, const std::uint8_t* data, Vector2u data_size)
    {
      glBindTexture(GL_TEXTURE_2D, texture_.get());

      glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y, data_size.x, data_size.y, GL_RGBA,
                      GL_UNSIGNED_BYTE, data);
    }

    Texture create_texture_from_image(const sf::Image& image)
    {
      return Texture(image);
    }

    TextureArray::TextureArray(GLuint name, Vector3u size)
      : texture_array_(name),
        texture_size_(size)
    {
    }


    GLint max_texture_size()
    {
      GLint result = 0;
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);

      return result;
    }
  }
}