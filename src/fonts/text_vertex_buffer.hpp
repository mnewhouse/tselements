/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef TEXT_VERTEX_BUFFER_HPP_289013891
#define TEXT_VERTEX_BUFFER_HPP_289013891

#include "graphics/texture.hpp"
#include "graphics/vertex_buffer.hpp"
#include "graphics/sampler.hpp"

#include "utility/vector2.hpp"
#include "utility/vertex.hpp"
#include "utility/color.hpp"

#include <boost/utility/string_ref.hpp>

#include <vector>

namespace ts
{
  namespace fonts
  {
    class BitmapFont;
    using GlyphVertex = Vertex<float, std::uint8_t>;

    class TextVertexBuffer
    {
    public:
      TextVertexBuffer();

      void clear();
      void add_text(boost::string_ref text, const BitmapFont& font, Vector2f position, Colorb color);
      void draw() const;

    private:
      struct Component
      {
        const graphics::Texture* texture;
        std::size_t buffer_offset;
        std::vector<GlyphVertex> vertices;
      };

      
      
      graphics::Buffer buffer_;

      mutable std::vector<Component> components_;
      mutable std::size_t buffer_size_;
      mutable bool dirty_ = false;
    };
  }
}

#endif