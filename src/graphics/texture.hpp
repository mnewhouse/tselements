/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/vector3.hpp"
#include "utility/rect.hpp"

#include "gl_check.hpp"

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
    
    class Texture
    {
    public:
      Texture() = default;
      explicit Texture(const sf::Image& image);
      explicit Texture(Vector2u texture_size);
      explicit Texture(GLuint texture, Vector2u texture_size);

      GLuint get() const { return texture_.get(); }
      Vector2u size() const { return texture_size_; }

      void update(Vector2i pos, const sf::Image& image);
      void update(Vector2i pos, const std::uint8_t* data, Vector2u data_size);

    private:
      std::unique_ptr<GLuint, detail::TextureDeleter> texture_;
      Vector2u texture_size_;
    };

    class TextureArray
    {
    public:
      TextureArray() = default;
      explicit TextureArray(GLuint name, Vector3u texture_size);

      GLuint get() const { return texture_array_.get(); }
      Vector3u size() const { return texture_size_; }

    private:
      std::unique_ptr<GLuint, detail::TextureDeleter> texture_array_;
      Vector3u texture_size_;
    };

    Texture create_texture_from_image(const sf::Image& image);
    GLint max_texture_size();

    inline Texture create_texture()
    {
      GLuint texture{};
      glCheck(glGenTextures(1, &texture));
      return Texture(texture, { 0, 0 });
    }
  }
}
