/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "utility/texture_atlas.hpp"

#include <algorithm>
#include <iterator>
#include <iostream>

namespace ts
{
  namespace utility
  {
    namespace detail
    {
      // Function template find_best_matching_rect
      // Finds the element in the range that has sufficient width and height,
      // and leaves the *least* space on the short side. That is, the rect
      // where min(rect.width - width, rect.height - height) is the lowest.
      // Returns an iterator to the best match.

      template <typename ForwardRectIt>
      static ForwardRectIt find_best_matching_rect(ForwardRectIt begin, ForwardRectIt end, std::int32_t width, std::int32_t height)
      {
        auto match_rect = [=](const IntRect& rect)
        {
          return rect.width >= width && rect.height >= height;
        };

        ForwardRectIt it = std::find_if(begin, end, match_rect);
        ForwardRectIt best_match = it;
        if (it != end)
        {
          auto best_fit = std::min(best_match->width - width, best_match->height - height);

          while (it != end)
          {
            auto fit = std::min(it->width - width, it->height - height);
            if (fit < best_fit)
            {
              best_match = it;
              best_fit = fit;
            }

            it = std::find_if(std::next(it), end, match_rect);
          }
        }

        return best_match;
      }

      // Function template split_intersecting_rects
      // Splits the rectangles in the range [it, end) as if `used_rect` would be cut out.
      // Writes the new rectangles into the output iterator `out`.
      template <typename ForwardRectIt, typename OutIt>
      static void split_intersecting_rects(ForwardRectIt it, ForwardRectIt end, IntRect used_rect, OutIt out)
      {
        auto right = used_rect.right();
        auto bottom = used_rect.bottom();

        for (; it != end; ++it)
        {
          IntRect free_rect = *it;
          if (used_rect.left < free_rect.right() && used_rect.right() > free_rect.left)
          {
            if (used_rect.top > free_rect.top && used_rect.top < free_rect.bottom())
            {
              auto out_rect = free_rect;
              out_rect.height = used_rect.top - free_rect.top;
              *out++ = out_rect;
            }

            if (bottom < free_rect.bottom())
            {
              auto out_rect = free_rect;
              out_rect.top = bottom;
              out_rect.height = free_rect.bottom() - bottom;
              *out++ = out_rect;
            }
          }

          if (used_rect.top < free_rect.bottom() && bottom > free_rect.top)
          {
            if (used_rect.left > free_rect.left && used_rect.left < free_rect.right())
            {
              auto out_rect = free_rect;
              out_rect.width = used_rect.left - free_rect.left;
              *out++ = out_rect;
            }

            // New node at the right side of the used node.
            if (right < free_rect.right())
            {
              auto out_rect = free_rect;
              out_rect.left = right;
              out_rect.width = free_rect.right() - right;
              *out++ = out_rect;
            }
          }
        }
      }

      // Function template remove_redundant_rects
      // Removes all rectangles in the range [begin_a, end_a) that are fully contained by any
      // rectangles in the same range.
      // Returns the new end of the range.
      template <typename BiDirRectIt>
      BiDirRectIt remove_redundant_rects(BiDirRectIt begin, BiDirRectIt end)
      {
        for (auto it = begin; it != end; )
        {
          const auto& rect = *it;

          if (std::any_of(std::next(it), end,
                          [=](const IntRect& other) { return contains(other, rect); }))
          {
            *it = std::move(*--end);
          }

          else
          {
            end = std::remove_if(++it, end, [=](const IntRect& other)
            {
              return contains(rect, other);
            });
          }
        }

        return end;
      }
    }

    TextureAtlas::TextureAtlas(Vector2i size)
      : my_size_(size)
    {
      if (size.x && size.y)
      {
        free_space_.push_back({ 0, 0, size.x, size.y });
      }
    }

    void TextureAtlas::clear()
    {
      *this = TextureAtlas(my_size_);
    }

    Vector2i TextureAtlas::size() const
    {
      return my_size_;
    }

    std::int32_t TextureAtlas::padding() const
    {
      return padding_;
    }

    void TextureAtlas::set_padding(std::int32_t padding)
    {
      padding_ = padding;
    }

    boost::optional<IntRect> TextureAtlas::insert(Vector2i rect_size)
    {
      const IntRect* free_space = free_space_.data();
      const IntRect* free_space_end = free_space_.data() + free_space_.size();

      auto padded_size = rect_size + padding_ * 2;

      const IntRect* best_match = detail::find_best_matching_rect(free_space, free_space_end, 
                                                                  rect_size.x, rect_size.y);
      if (best_match == free_space_end)
      {
        // No suitable rects found.
        return boost::none;
      }

      IntRect used_rect(best_match->left, best_match->top, rect_size.x, rect_size.y);
      IntRect padded_rect = used_rect;
      padded_rect.width += padding_;
      padded_rect.height += padding_;

      // Split the free rects. First we need to find all the rects that intersect with the one
      // we allocated, split them, and then remove the old ones.
      auto intersect_it = std::partition(free_space_.begin(), free_space_.end(),
                                         [padded_rect](const IntRect& rect)
      {
        return !intersects(rect, padded_rect);
      });

      rect_cache_.clear();
      detail::split_intersecting_rects(intersect_it, free_space_.end(), padded_rect, std::back_inserter(rect_cache_));

      // By reserving, we make sure nothing will throw from now, leaving ourselves in a valid state
      // in the face of exceptions.

      std::size_t intersect_index = std::distance(free_space_.begin(), intersect_it);
      free_space_.reserve(free_space_.size() + rect_cache_.size());     

      free_space_.erase(free_space_.begin() + intersect_index, free_space_.end());
      free_space_.insert(free_space_.end(), rect_cache_.begin(), rect_cache_.end());     
      free_space_.erase(detail::remove_redundant_rects(free_space_.begin(), free_space_.end()), free_space_.end());

      return used_rect;
    }


    AtlasList::AtlasList(Vector2i atlas_size)
      : atlas_size_(atlas_size)
    {
      atlas_list_.emplace_back(atlas_size);
    }

    std::size_t AtlasList::current_atlas() const
    {
      return current_atlas_;
    }

    std::size_t AtlasList::atlas_count() const
    {
      return atlas_list_.size();
    }

    Vector2i AtlasList::atlas_size() const
    {
      return atlas_size_;
    }

    Vector2i AtlasList::atlas_size(std::size_t atlas_id) const
    {
      return atlas_list_[atlas_id].size();
    }

    std::int32_t AtlasList::padding() const
    {
      return padding_;
    }

    void AtlasList::set_padding(std::int32_t padding)
    {
      padding_ = padding;
    }

    std::int32_t AtlasList::fragment_overlap() const
    {
      return fragment_overlap_;
    }

    void AtlasList::set_fragment_overlap(std::int32_t fragment_overlap)
    {
      fragment_overlap_ = fragment_overlap;
    }

    boost::optional<AtlasEntry> AtlasList::allocate_rect(IntRect source_rect)
    {     
      if (auto rect = atlas_list_[current_atlas_].insert({ source_rect.width, source_rect.height }))
      {
        AtlasEntry result;
        result.atlas_id = current_atlas_;
        result.atlas_rect = *rect;
        result.source_rect = source_rect;
        return result;
      }

      return boost::none;
    };

    std::size_t AtlasList::create_atlas()
    {
      return create_atlas(atlas_size_);
    }

    std::size_t AtlasList::create_atlas(Vector2i size)
    {
      current_atlas_ = atlas_list_.size();
      atlas_list_.emplace_back(atlas_size_);
      atlas_list_.back().set_padding(padding_);

      return current_atlas_;
    }
  }
}