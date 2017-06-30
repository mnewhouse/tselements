/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <GL/glew.h>
#include <GL/GL.h>

#include "graphics/gl_check.hpp"

#include <memory>

namespace ts
{
  namespace graphics
  {
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

    inline VertexArray create_vertex_array()
    {
      GLuint vao;
      glCheck(glGenVertexArrays(1, &vao));
      return VertexArray(vao);
    }

    inline void bind_array_buffer(GLuint b)
    {
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, b));
    }

    inline void bind_array_buffer(const Buffer& b)
    {
      bind_array_buffer(b.get());
    }

    inline void bind_element_array_buffer(GLuint b)
    {
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b));
    }

    inline void bind_element_array_buffer(const Buffer& b)
    {
      bind_element_array_buffer(b.get());
    }
  }
}
