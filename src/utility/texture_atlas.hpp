/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/rect.hpp"
#include "utility/vector2.hpp"

#include <boost/optional.hpp>
#include <boost/container/small_vector.hpp>

#include <vector>

namespace ts
{
  namespace utility
  {
    class TextureAtlas
    {
    public:
      explicit TextureAtlas(Vector2i size = {});

      Vector2i size() const;
      void clear();
      boost::optional<IntRect> insert(Vector2i rect_size);

      void set_padding(std::int32_t padding);
      std::int32_t padding() const;

    private:
      std::vector<IntRect> free_space_;
      std::vector<IntRect> rect_cache_;

      std::int32_t padding_ = 1;
      Vector2i my_size_ = {};
    };

    struct AtlasEntry
    {
      IntRect source_rect;
      IntRect atlas_rect;
      std::size_t atlas_id;
    };

    class AtlasList
    {
    public:
      explicit AtlasList(Vector2i atlas_size);

      std::size_t current_atlas() const;
      std::size_t create_atlas();

      boost::optional<AtlasEntry> allocate_rect(IntRect source_rect);

      template <typename OutIt>
      void allocate_fragmented_rect(IntRect source_rect, OutIt out);

      std::size_t atlas_count() const;
      Vector2i atlas_size() const;
      Vector2i atlas_size(std::size_t atlas_id) const;
      void set_atlas_size(Vector2i size);

      void set_padding(std::int32_t padding);
      std::int32_t padding() const;

      void set_fragment_overlap(std::int32_t overlap);
      std::int32_t fragment_overlap() const;

    private:
      std::size_t create_atlas(Vector2i size);

      std::size_t current_atlas_ = 0;
      Vector2i atlas_size_;
      std::int32_t padding_ = 1;
      std::int32_t fragment_overlap_ = 0;
      std::vector<utility::TextureAtlas> atlas_list_;
    };


    template <typename OutIt>
    void AtlasList::allocate_fragmented_rect(IntRect source_rect, OutIt out)
    {
      std::size_t active_atlas = current_atlas_;

      // Divide the rect into atlas-sized sub-rectangles
      for (auto y = 0; y < source_rect.height; y += atlas_size_.y - fragment_overlap_)
      {
        for (auto x = 0; x < source_rect.width; x += atlas_size_.x - fragment_overlap_)
        {
          Vector2i size =
          {
            std::min(atlas_size_.x, source_rect.width - x),
            std::min(atlas_size_.y, source_rect.height - y)
          };

          create_atlas(size);
          *out++ = *allocate_rect({ x, y, size.x, size.y });
        }
      }

      // Reset the current atlas to the previously active one.
      current_atlas_ = active_atlas;
    }

    // The following function can be called as a generic allocation function.
    // It resorts to fragmented allocation when the source rect exceeds the maximum size
    // as specified, and calls the unary callback function with any AtlasEntry objects
    // to be processed.
    template <typename EntryCallback>
    static void allocate_atlas_rect(AtlasList& atlas_list, IntRect source_rect, Vector2i max_size,
                                    EntryCallback&& callback)
    {
      auto atlas_size = atlas_list.atlas_size();
      max_size.x = std::min(atlas_size.x, max_size.x);
      max_size.y = std::min(atlas_size.y, max_size.y);

      if (max_size.x < source_rect.width || max_size.y < source_rect.height)
      {
        boost::container::small_vector<utility::AtlasEntry, 8> entries;
        atlas_list.allocate_fragmented_rect(source_rect, std::back_inserter(entries));

        for (const auto& entry : entries)
        {
          callback(entry);
        }
      }

      else
      {
        auto entry = atlas_list.allocate_rect(source_rect);
        if (!entry)
        {
          atlas_list.create_atlas();
          entry = atlas_list.allocate_rect(source_rect);
        }

        callback(*entry);
      }
    }
  }
}
