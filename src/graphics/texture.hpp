/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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

#include <gli/texture2d.hpp>
#include <gli/texture2d_array.hpp>

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
      explicit Texture(Vector2u texture_size, std::uint32_t num_levels = 1, std::uint32_t num_layers = 1);
      explicit Texture(GLuint texture, Vector2u texture_size,
                       std::uint32_t num_levels = 1, std::uint32_t num_layers = 1);

      GLuint get() const { return texture_.get(); }
      Vector2u size() const { return texture_size_; }

      GLenum target() const { return target_; };
      std::uint32_t layers() const { return num_layers_; }
      std::uint32_t levels() const { return num_levels_; }

    private:
      std::unique_ptr<GLuint, detail::TextureDeleter> texture_;      
      Vector2u texture_size_;
      std::uint32_t num_layers_;
      std::uint32_t num_levels_;
      GLenum target_ = GL_TEXTURE_2D;
    };

    Texture create_texture(const sf::Image& image);
    Texture create_texture(const gli::texture2d& texture);
    Texture create_texture_array(const gli::texture2d* layers, std::size_t num_layers);

    GLint max_texture_size();
  }
}
