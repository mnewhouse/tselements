/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <GL/glew.h>

#include <memory>

namespace ts
{
  namespace graphics
  {
    namespace detail
    {
      struct SamplerDeleter
      {
        using pointer = GLuint;
        void operator()(pointer sampler) const
        {
          glDeleteSamplers(1, &sampler);
        }
      };
    }

    using Sampler = std::unique_ptr<GLuint, detail::SamplerDeleter>;
  }
}
