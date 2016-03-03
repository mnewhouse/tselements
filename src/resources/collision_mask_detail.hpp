/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef COLLISION_MASK_DETAIL_HPP_58598122
#define COLLISION_MASK_DETAIL_HPP_58598122

#include "collision_mask.hpp"
#include "pattern.hpp"

#include "utility/vector2.hpp"
#include "utility/rotation.hpp"
#include "utility/transform.hpp"

namespace ts
{
  namespace resources
  {
    namespace collision_mask
    {
      inline constexpr std::uint32_t word_bits()
      {
        using bitmask_type = CollisionMask::bitmask_type;
        return sizeof(bitmask_type) * CHAR_BIT;
      }

      inline constexpr std::uint32_t compute_word_count(std::uint32_t real_size)
      {
        return (real_size + word_bits() - 1) / word_bits();
      }

      inline constexpr double compute_rotation_multiplier(std::uint32_t frame_count)
      {
        return static_cast<double>(frame_count) / 360.0;
      }

      template <typename WallTest>
      auto create_static_collision_mask(const resources::Pattern& pattern, IntRect rect,
                                        std::uint32_t row_words, std::uint32_t level_count, WallTest&& wall_test)
      {
        auto pattern_size = make_vector2<std::uint32_t>(rect.width, rect.height);
        constexpr auto word_bits = collision_mask::word_bits();

        using bitmask_type = CollisionMask::bitmask_type;
        std::vector<bitmask_type> result(row_words * pattern_size.y * level_count);
        auto data_ptr = result.data();

        for (std::uint32_t level = 0; level != level_count; ++level)
        {
          for (std::uint32_t y = 0; y != pattern_size.y; ++y)
          {
            auto pattern_ptr = pattern.row_begin(y + rect.top) + rect.left;
            

            for (std::uint32_t x = 0; x != pattern_size.x; )
            {
              bitmask_type word = 0;
              bitmask_type bit = 1;

              for (std::uint32_t x_end = std::min(x + word_bits, pattern_size.x); x != x_end; ++x, ++pattern_ptr)
              {
                if (wall_test(*pattern_ptr, level))
                {
                  word |= bit;
                }

                bit <<= 1;
              }

              *data_ptr++ = word;
            }            
          }
        }

        return result;
      }

      template <typename WallTest>
      auto create_dynamic_collision_mask(const resources::Pattern& pattern, IntRect rect, 
                                         std::uint32_t row_words, std::uint32_t frame_count, WallTest&& wall_test)
      {
        auto pattern_size = make_vector2<std::uint32_t>(rect.width, rect.height);
        constexpr auto word_bits = collision_mask::word_bits();

        using bitmask_type = CollisionMask::bitmask_type;

        const double frame_domain = 360.0 / frame_count;

        auto dest_size = make_vector2(row_words * word_bits, std::max(pattern_size.x, pattern_size.y));
        auto dest_center = make_vector2(dest_size.x * 0.5, dest_size.y * 0.5);

        auto source_center = vector2_cast<double>(pattern_size) * 0.5;

        auto frame_size = row_words * dest_size.y;
        std::vector<bitmask_type> result(frame_size * frame_count);

        for (std::uint32_t frame_id = 0; frame_id != frame_count; ++frame_id)
        {
          auto frame_ptr = result.data() + frame_id * frame_size;
          auto frame_rotation = degrees(frame_domain * frame_id);

          auto sin = -std::sin(frame_rotation.radians());
          auto cos = std::cos(frame_rotation.radians());

          for (std::uint32_t dest_y = 0; dest_y != dest_size.y; ++dest_y)
          {
            for (std::uint32_t dest_x = 0, word_index = 0; dest_x != dest_size.x; ++word_index)
            {
              bitmask_type bit = 1;
              bitmask_type word = 0;

              for (std::uint32_t x_end = std::min(dest_x + word_bits, dest_size.x); dest_x != x_end; ++dest_x, bit <<= 1)
              {
                auto dest_point = vector2_cast<double>(make_vector2(dest_x, dest_y));
                auto source_point = transform_point(dest_point - dest_center, sin, cos) + source_center;

                if (source_point.x >= 0.0 && source_point.y >= 0.0)
                {
                  auto point = vector2_cast<std::uint32_t>(source_point + 0.5);
                  if (point.x < pattern_size.x && point.y < pattern_size.y &&
                      wall_test(pattern(point.x, point.y)))
                  {
                    word |= bit;
                  }
                }
              }

              auto word_ptr = frame_ptr + dest_y * row_words + word_index;
              *word_ptr = word;
            }
          }
        }

        return result;
      }
    }

    template <typename WallTest>
    CollisionMask::CollisionMask(const resources::Pattern& pattern, IntRect rect,
                                 std::uint32_t level_count, WallTest&& wall_test)
      : row_width_(collision_mask::compute_word_count(pattern.size().x)),
        frame_count_(level_count),
        bitmap_size_(row_width_ * collision_mask::word_bits(), pattern.size().y),
        pixel_data_(collision_mask::create_static_collision_mask(pattern, rect, row_width_, level_count, 
                                                                 std::forward<WallTest>(wall_test))),
        rotation_multiplier_(collision_mask::compute_rotation_multiplier(level_count))
    {
    }

    template <typename WallTest>
    CollisionMask::CollisionMask(const resources::Pattern& pattern, std::uint32_t level_count, WallTest&& wall_test)
      : CollisionMask(pattern, IntRect(0, 0, pattern.size().x, pattern.size().y), level_count, std::forward<WallTest>(wall_test))
    {
    }

    template <typename WallTest>
    CollisionMask::CollisionMask(dynamic_mask_t, const resources::Pattern& pattern, IntRect rect,
                                 std::uint32_t frame_count, WallTest&& wall_test)
      : row_width_(collision_mask::compute_word_count(std::max(pattern.size().x, pattern.size().y))),
        frame_count_(frame_count),
        bitmap_size_(row_width_ * collision_mask::word_bits(), std::max(pattern.size().x, pattern.size().y)),
        pixel_data_(collision_mask::create_dynamic_collision_mask(pattern, rect, row_width_, frame_count, 
                                                                  std::forward<WallTest>(wall_test))),
        rotation_multiplier_(collision_mask::compute_rotation_multiplier(frame_count))
    {
    }

    template <typename WallTest>
    CollisionMask::CollisionMask(dynamic_mask_t, const resources::Pattern& pattern,
                                 std::uint32_t frame_count, WallTest&& wall_test)
      : CollisionMask(dynamic_mask, pattern, IntRect(0, 0, pattern.size().x, pattern.size().y),
                      frame_count, std::forward<WallTest>(wall_test))
    {
    }

    inline bool CollisionMask::FrameInterface::operator()(std::uint32_t x, std::uint32_t y) const
    {
      auto word_ptr = row_begin(y) + x / collision_mask::word_bits();

      return (*word_ptr & (bitmask_type(1) << (x & (collision_mask::word_bits() - 1)))) != 0;
    }

    inline const CollisionMask::bitmask_type* CollisionMask::FrameInterface::row_begin(std::uint32_t y) const
    {
      return frame_bits_ + y * row_width_;
    }

    inline std::size_t CollisionMask::FrameInterface::row_count() const
    {
      return row_count_;
    }

    inline std::size_t CollisionMask::FrameInterface::row_width() const
    {
      return row_width_ * collision_mask::word_bits();
    }
  }
}

#endif