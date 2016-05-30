/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "text_vertex_buffer.hpp"
#include "bitmap_font.hpp"

#include "utility/utf8.h"

#include <numeric>

namespace ts
{
  namespace fonts
  {
    namespace detail
    {
      void add_vertices(const GlyphInfo& glyph_info, Vector2u texture_size, 
                        Vector2f position, Colorb color, std::int32_t kerning,
                        std::vector<GlyphVertex>& vertices)
      {
        auto top_left = position + glyph_info.offset;
        top_left.x += static_cast<float>(kerning);

        auto bottom_right = top_left + glyph_info.size;

        auto inverse_texture_size = 1.0f / vector2_cast<float>(texture_size);
        
        auto tex_top_left = glyph_info.position * inverse_texture_size;     
        auto tex_bottom_right = tex_top_left + glyph_info.size * inverse_texture_size.x;

        GlyphVertex temp_verts[4] = 
        {
          { top_left, tex_top_left, color},
          { { top_left.x, bottom_right.y }, { tex_top_left.x, tex_bottom_right.y }, color },
          { { bottom_right.x, top_left.y }, { tex_bottom_right.x, tex_top_left.y }, color },
          { bottom_right, tex_bottom_right, color }
        };

        vertices.insert(vertices.end(), temp_verts, temp_verts + 3);
        vertices.insert(vertices.end(), temp_verts + 1, temp_verts + 4);
      }
    }

    TextVertexBuffer::TextVertexBuffer()
    {
      GLuint name;
      glGenBuffers(1, &name);
      buffer_.reset(name);
    }

    void TextVertexBuffer::clear()
    {
      components_.clear();
    }

    void TextVertexBuffer::add_text(boost::string_ref text, const BitmapFont& font, Vector2f position, Colorb color)
    {
      using utf8_iterator = utf8::iterator<const char*>;
      utf8_iterator utf8_it(text.begin(), text.begin(), text.end());
      utf8_iterator utf8_end(text.end(), text.begin(), text.end());

      std::uint32_t last_page_index = 0;
      Component* component_ptr = nullptr;

      std::uint32_t last_cp = 0;
      for (; utf8_it != utf8_end; ++utf8_it)
      {
        auto code_point = *utf8_it;
        if (auto glyph_info = font.glyph_info(code_point))
        {
          auto page_index = glyph_info->page;
          if (!component_ptr || last_page_index != page_index)
          {
            // Find the matching component. If there is none, create a new component with the required
            // parameters, i.e. texture and atlas id.
            const auto texture_ptr = &font.page_texture(page_index);
            auto it = std::find_if(components_.begin(), components_.end(),
                                   [=](const Component& component)
            {
              return component.texture == texture_ptr;
            });

            if (it != components_.end())
            {
              component_ptr = &*it;
            }

            else
            {
              Component component;
              component.texture = texture_ptr;
              component.buffer_offset = 0;

              components_.push_back(std::move(component));
              component_ptr = &components_.back();
            }

            last_page_index = page_index;
          }

          auto kerning = last_cp != 0 ? font.kerning(last_cp, code_point) : 0;

          detail::add_vertices(*glyph_info, component_ptr->texture->size(), 
                               position, color, kerning, component_ptr->vertices);

          dirty_ = true;
          position.x += glyph_info->x_advance + kerning;

          last_cp = code_point;
        }
      }
    }


    void TextVertexBuffer::draw() const
    {
      if (dirty_)
      {
        glBindBuffer(GL_ARRAY_BUFFER, buffer_.get());

        auto total_vertex_count = std::accumulate(components_.begin(), components_.end(), std::size_t(0),
                                                  [](std::size_t acc, const Component& component)
        {
          return acc + component.vertices.size();
        });

        buffer_size_ = graphics::next_power_of_two(total_vertex_count * sizeof(GlyphVertex));
        glBufferData(GL_ARRAY_BUFFER, buffer_size_,
                     nullptr, GL_STATIC_DRAW);

        std::size_t offset = 0;
        for (auto& component : components_)
        {
          component.buffer_offset = offset;
          auto size = component.vertices.size() * sizeof(GlyphVertex);
          glBufferSubData(GL_ARRAY_BUFFER, offset, size, component.vertices.data());

          offset += size;
        }

        dirty_ = false;
      }

      glBindBuffer(GL_ARRAY_BUFFER, buffer_.get());

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex),
                            reinterpret_cast<void*>(offsetof(GlyphVertex, position)));

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex),
                            reinterpret_cast<void*>(offsetof(GlyphVertex, texture_coords)));

      glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GlyphVertex),
                            reinterpret_cast<void*>(offsetof(GlyphVertex, color)));

      glActiveTexture(GL_TEXTURE0);
      for (const auto& component : components_)
      {
        auto texture = component.texture;

        glBindTexture(GL_TEXTURE_2D, component.texture->get());
        glDrawArrays(GL_TRIANGLES, component.buffer_offset, component.vertices.size());
      }

      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);
      glDisableVertexAttribArray(2);
    }
  }
}