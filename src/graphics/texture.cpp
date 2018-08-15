/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "texture.hpp"

#include <SFML/Graphics/Image.hpp>

#include <gli/gl.hpp>

namespace ts
{
  namespace graphics
  {
    static GLuint gen_texture()
    {
      GLuint name{};
      glGenTextures(1, &name);
      return name;
    }

    Texture::Texture(Vector2u texture_size, std::uint32_t num_levels, std::uint32_t num_layers)
      : Texture(gen_texture(), texture_size, num_levels, num_layers)
    {
    }

    Texture::Texture(GLuint texture, Vector2u texture_size, std::uint32_t num_levels, std::uint32_t num_layers)
      : texture_(texture),
        texture_size_(texture_size),
        num_levels_(num_levels),
        num_layers_(num_layers)
    {
      if (num_layers >= 2)
      {
        target_ = GL_TEXTURE_2D_ARRAY;
      }
    }

    /*
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
    */


    graphics::Texture create_texture(const gli::texture2d& tex_2d)
    {
      gli::gl gl(gli::gl::PROFILE_GL32);
      const auto format = gl.translate(tex_2d.format(), tex_2d.swizzles());
      const auto base_extent = tex_2d.extent(tex_2d.base_level());

      graphics::Texture result(Vector2u(base_extent.x, base_extent.y), tex_2d.levels(), 1);
      const auto name = result.get();
      const auto target = result.target();

      glBindTexture(target, name);
      glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(tex_2d.levels() - 1));
      glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
      glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
      glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
      glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);

      glTexStorage2D(target, static_cast<GLint>(tex_2d.levels()), format.Internal, base_extent.x, base_extent.y);

      for (std::size_t level = 0; level < tex_2d.levels(); ++level)
      {
        auto extent = tex_2d.extent(level);
        if (gli::is_compressed(tex_2d.format()))
        {
          glCompressedTexSubImage2D(target, static_cast<GLint>(level), 0, 0, extent.x, extent.y,
                                    format.Internal, static_cast<GLsizei>(tex_2d.size(level)),
                                    tex_2d.data(0, 0, level));
        }

        else
        {
          glTexSubImage2D(target, static_cast<GLint>(level), 0, 0, extent.x, extent.y,
                          format.External, format.Type, tex_2d.data(0, 0, level));
        }
      }

      return result;
    }

    graphics::Texture create_texture(const gli::texture2d* layers, std::size_t num_layers)
    {
      if (num_layers == 0) return{};

      gli::gl gl(gli::gl::PROFILE_GL32);      
      const auto base_extent = layers->extent(layers->base_level());
      graphics::Texture result(Vector2u(base_extent.x, base_extent.y), layers->levels(), num_layers);

      const auto name = result.get();
      const auto target = result.target();
      const auto format = gl.translate(layers->format(), layers->swizzles());

      glBindTexture(target, name);
      glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, layers->levels());
      glTexStorage3D(target, layers->levels(), format.Internal, base_extent.x, base_extent.y, num_layers);

      for (std::size_t layer = 0; layer != num_layers; ++layer)
      {
        auto& tex = layers[layer];
        for (std::size_t level = 0; level != tex.levels(); ++level)
        {
          auto extent = tex.extent(level);
          auto layer_format = gl.translate(tex.format(), tex.swizzles());

          if (gli::is_compressed(tex.format()))
          {
            glCompressedTexSubImage3D(target, static_cast<GLint>(level), 0, 0, 0, extent.x, extent.y, layer,
                                      format.Internal, static_cast<GLsizei>(tex.size(level)),
                                      tex.data(0, 0, level));
          }

          else
          {
            glTexSubImage3D(target, static_cast<GLint>(level), 0, 0, 0, extent.x, extent.y, layer,
                            layer_format.External, layer_format.Type,
                            tex.data(0, 0, level));
          }
        }        
      }

      glBindTexture(target, 0);
      return result;
    }

    Texture create_texture(const sf::Image& image)
    {
      Vector2u size = { image.getSize().x, image.getSize().y };
      Texture tex(size, 1, 1);

      auto name = tex.get();
      auto target = tex.target();

      glBindTexture(target, name);
      glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
      glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
      glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexStorage2D(target, 1, GL_RGBA8, size.x, size.y);            

      glTexSubImage2D(target, 0, 0, 0, size.x, size.y,
                      GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
      glBindTexture(target, 0);

      return tex;
    }    

    GLint max_texture_size()
    {
      GLint result = 0;
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);

      return result;
    }
  }
}