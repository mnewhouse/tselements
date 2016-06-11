/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "text_geometry.hpp"
#include "bitmap_font.hpp"

#include "utility/utf8.h"

#include <boost/optional.hpp>

#include <numeric>

namespace ts
{
  namespace fonts
  {
    namespace detail
    {
      template <typename CachedComponent>
      FloatRect add_vertices(const GlyphInfo& glyph_info, const graphics::Texture& texture,
                             Vector2f position, Colorb color, std::int32_t kerning,
                             std::vector<CachedComponent>& component_cache,
                             CachedComponent*& component_hint)
      {
        auto top_left = position + glyph_info.offset;
        top_left.x += static_cast<float>(kerning);

        auto bottom_right = top_left + glyph_info.size;
        auto inverse_texture_size = 1.0f / vector2_cast<float>(texture.size());
        
        auto tex_top_left = glyph_info.position * inverse_texture_size;     
        auto tex_bottom_right = tex_top_left + glyph_info.size * inverse_texture_size.x;

        graphics::Geometry::vertex_type temp_verts[4] = 
        {
          { top_left, tex_top_left, color},
          { { top_left.x, bottom_right.y }, { tex_top_left.x, tex_bottom_right.y }, color },
          { { bottom_right.x, top_left.y }, { tex_bottom_right.x, tex_top_left.y }, color },
          { bottom_right, tex_bottom_right, color }
        };

        if (!component_hint || component_hint->texture == &texture)
        {
          auto texture_ptr = &texture;
          auto it = std::find_if(component_cache.begin(), component_cache.end(),
                                 [=](const auto& component)
          {
            return component.texture == texture_ptr;
          });

          if (it != component_cache.end())
          {
            component_hint = &*it;
          }

          else
          {
            component_cache.emplace_back();
            component_hint = &component_cache.back();
            component_hint->texture = texture_ptr;
          }
        }

        auto& vertices = component_hint->vertices;
        vertices.insert(vertices.end(), temp_verts, temp_verts + 3);
        vertices.insert(vertices.end(), temp_verts + 1, temp_verts + 4);

        return make_rect_from_points(top_left, bottom_right);
      }
    }

    void TextGeometry::add_text(boost::string_ref text, const BitmapFont& font, FloatRect rect, Colorb color,
                                    std::uint32_t flags)
                                    
    {
      using utf8_iterator = utf8::iterator<const char*>;
      utf8_iterator utf8_it(text.begin(), text.begin(), text.end());
      utf8_iterator utf8_end(text.end(), text.begin(), text.end());

      std::uint32_t last_page_index = 0;
      CacheComponent* hint = nullptr;

      for (auto& component : vertex_cache_)
      {
        component.vertices.clear();
      }

      boost::optional<FloatRect> total_bounds;
      auto position = make_vector2(rect.left, rect.top);

      std::uint32_t last_cp = 0;
      for (; utf8_it != utf8_end; ++utf8_it)
      {
        auto code_point = *utf8_it;
        if (auto glyph_info = font.glyph_info(code_point))
        {
          auto page_index = glyph_info->page;
          const auto& texture = font.page_texture(page_index);

          auto kerning = font.kerning(last_cp, code_point);
          auto bounds = detail::add_vertices(*glyph_info, texture,
                                             position, color, kerning,
                                             vertex_cache_, hint);

          if (!total_bounds) total_bounds.emplace(bounds);
          else *total_bounds = combine(*total_bounds, bounds);
          
          position.x += glyph_info->x_advance + kerning;
          last_cp = code_point;
        }
      }

      Vector2f offset;
      if (flags & fonts::text_style::center_horizontal)
      {
        offset.x = (rect.width - total_bounds->width) * 0.5f;
      }

      if (flags & fonts::text_style::center_vertical)
      {
        offset.y = (rect.height - total_bounds->height) * 0.5f;
      }

      for (auto& component : vertex_cache_)
      {
        if (component.vertices.empty()) continue;

        if (offset.x != 0.0f || offset.y != 0.0f)
        {
          for (auto& vertex : component.vertices)
          {
            vertex.position += offset;
          }
        }

        add_vertices(component.vertices.data(), component.vertices.size(), component.texture);
      }
    }
  }
}