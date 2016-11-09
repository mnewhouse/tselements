/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef VERTEX_ARRAY_HPP_3312950
#define VERTEX_ARRAY_HPP_3312950

#include <GL/glew.h>
#include <GL/GL.h>

#include "graphics/gl_check.hpp"

#include <memory>

namespace ts
{
  namespace graphics
  {
    inline std::size_t next_power_of_two(std::size_t n)
    {
      std::size_t result = 1;
      while (result < n) result <<= 1;

      return result;
    }

    namespace detail
    {
      struct VertexArrayDeleter
      {
        using pointer = GLuint;
        void operator()(pointer vertex_array) const
        {
          glDeleteVertexArrays(1, &vertex_array);
        }
      };

      struct BufferDeleter
      {
        using pointer = GLuint;
        void operator()(pointer buffer) const
        {
          glDeleteBuffers(1, &buffer);
        }
      };

      template <typename T>
      struct MappedBufferDeleter
      {
        struct pointer
        {
          friend bool operator==(const pointer& a, const pointer& b)
          {
            return a.ptr == b.ptr;
          }

          friend bool operator!=(const pointer& a, const pointer& b)
          {
            return a.ptr != b.ptr;
          }

          GLuint buffer = 0;
          T* ptr = nullptr;
        };

        void operator()(pointer m) const
        {
          if (m.ptr)
          {
            glUnmapNamedBuffer(m.buffer);
          }
        }
      };
    }

    using VertexArray = std::unique_ptr<GLuint, detail::VertexArrayDeleter>;
    using Buffer = std::unique_ptr<GLuint, detail::BufferDeleter>;

    template <typename T>
    using MappedBuffer = std::unique_ptr<GLuint, detail::MappedBufferDeleter<T>>;

    template <typename T>
    MappedBuffer<T> mapped_buffer(GLuint buffer, T* ptr)
    {
      using buffer_handle = typename detail::MappedBufferDeleter<T>::pointer;
      buffer_handle handle;
      handle.ptr = ptr;
      handle.buffer = buffer;
      return MappedBuffer<T>(handle);
    }

    inline Buffer create_buffer()
    {
      GLuint buffer;
      glCheck(glGenBuffers(1, &buffer));
      return Buffer(buffer);
    }
  }
}

#endif