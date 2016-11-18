/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "buffer.hpp"
#include "texture.hpp"

#include "utility/vertex.hpp"

#include <vector>

namespace ts
{
  namespace graphics
  {
    class Geometry
    {
    public:
      Geometry();
      using vertex_type = Vertex<float, std::uint8_t>;

      void clear();
      void draw() const;

      using component_hint = void*;
      component_hint add_vertices(const vertex_type* vertices, std::size_t count,
                                  const Texture* texture, component_hint hint = nullptr);

    private:
      struct Component
      {
        const Texture* texture;
        std::size_t buffer_offset;
        std::vector<vertex_type> vertices;
      };

      Buffer buffer_;
      Texture dummy_texture_;

      mutable std::vector<Component> components_;
      mutable std::size_t buffer_size_ = 0;
      mutable bool dirty_ = false;
    };
  }
}
