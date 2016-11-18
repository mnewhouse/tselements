/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "collision_mask.hpp"
#include "collision_mask_detail.hpp"

#include <stdexcept>
#include <algorithm>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    CollisionMask::FrameInterface::FrameInterface(const bitmask_type* bits, std::size_t row_width,
                                                  std::size_t row_count)
      : frame_bits_(bits),
        row_width_(row_width),
        row_count_(row_count)
    {
    }

    CollisionMask::FrameInterface CollisionMask::frame(std::uint32_t frame_id) const
    {
      auto data_ptr = pixel_data_.data() + bitmap_size_.y * row_width_ * frame_id;

      return FrameInterface(data_ptr, row_width_, bitmap_size_.y);
    }

    CollisionMask::FrameInterface CollisionMask::rotation_frame(Rotation<double> rotation) const
    {
      auto deg = normalize(rotation).degrees();
      if (deg < 0.0) deg += 360.0;

      // Calculate the frame id that matches the given rotation.
      auto frame_id = static_cast<std::uint32_t>(deg * rotation_multiplier_ + 0.5);
      if (frame_id >= frame_count_)
      {
        frame_id -= frame_count_;
      }

      return frame(frame_id);
    }

    IntRect CollisionMask::bounding_box() const
    {
      return bounding_box_;
    }

    namespace detail
    {
      static CollisionPoint compute_collision_point(CollisionMask::bitmask_type word, Vector2i word_position)
      {
        // Get the first bit that is set, and add the bit index to the x position.
        for (std::int32_t bit = 0; bit != 32; ++bit)        
        {
          if (word & 1)
          {
            word_position.x += bit;
            return CollisionPoint(word_position);
          }

          word >>= 1;
        }

        return CollisionPoint();
      }

      static CollisionPoint test_collision_impl(const CollisionMaskFrame& subject, const CollisionMaskFrame& object,
                                                Vector2i subject_position, Vector2i object_position, 
                                                Vector2i subject_center, Vector2i object_center,
                                                IntRect intersect_area)
      {
        // This function assumes that subject_position.left is aligned to the same
        // bit index as the intersection area. We can use this knowledge to write a reasonably efficient algorithm.
        constexpr auto word_bits = collision_mask::word_bits();

        auto subject_row = intersect_area.top - subject_position.y + subject_center.y;
        auto object_row = intersect_area.top - object_position.y + object_center.y;

        auto object_bit_index = intersect_area.left - object_position.x + object_center.x;
        auto object_word_index = static_cast<std::uint32_t>(object_bit_index) / word_bits;        

        auto object_mask_offset = object_bit_index & (word_bits - 1);
        auto object_inverse_mask_offset = word_bits - object_mask_offset;

        auto word_count = intersect_area.width / word_bits;
        if (object_mask_offset != 0)
        {
          if (word_count == 0)
          {
            for (std::int32_t row = 0; row != intersect_area.height; ++row, ++subject_row, ++object_row)
            {
              auto subject_ptr = subject.row_begin(subject_row);
              auto object_ptr = object.row_begin(object_row) + object_word_index;

              auto object_mask = *object_ptr++ >> object_mask_offset;
              if (auto collision = *subject_ptr & object_mask)
              {
                // We collided, calculate the point of collision and return it.
                auto word_position = make_vector2(intersect_area.left, intersect_area.top + row);

                return compute_collision_point(collision, word_position);
              }
            }
          }

          else
          {
            for (std::int32_t row = 0; row != intersect_area.height; ++row, ++subject_row, ++object_row)
            {
              auto subject_ptr = subject.row_begin(subject_row);
              auto object_ptr = object.row_begin(object_row) + object_word_index;

              auto object_mask = *object_ptr++ >> object_mask_offset;
              for (std::int32_t word_index = 0; word_index != word_count; ++word_index, ++object_ptr, ++subject_ptr)
              {
                auto object_word = *object_ptr;
                object_mask |= object_word << object_inverse_mask_offset;

                if (auto collision = *subject_ptr & object_mask)
                {
                  // We collided, calculate the point of collision and return it.
                  auto word_position = make_vector2(intersect_area.left, intersect_area.top + row);

                  return compute_collision_point(collision, word_position);
                }

                // The object bits that we couldn't use in this iteration must be set for the next one.
                object_mask = object_word >> object_mask_offset;
              }
            }
          }
        }

        else
        {
          // When the subject and object alignments match, we can use a simpler and more efficient algorithm
          // than when they don't.
          for (std::int32_t row = 0; row != intersect_area.height; ++row, ++subject_row, ++object_row)
          {
            auto subject_ptr = subject.row_begin(subject_row);
            auto object_ptr = object.row_begin(object_row) + object_word_index;

            for (auto word_index = 0; word_index != word_count; ++word_index, ++subject_ptr, ++object_ptr)
            {
              if (auto collision = *subject_ptr & *object_ptr)
              {
                auto word_position = make_vector2(intersect_area.left, intersect_area.top + row);

                return compute_collision_point(collision, word_position);
              }
            }
          }
        }

        // No collision.
        return CollisionPoint();
      }

      static CollisionPoint test_collision(const CollisionMaskFrame& subject, const CollisionMaskFrame& object,
                                           Vector2i subject_size, Vector2i object_size,
                                           Vector2i subject_position, Vector2i object_position,
                                           Vector2i subject_center, Vector2i object_center)
      {
        // Find the intersection of subject and object areas, then
        // test for collision in that area.
        IntRect subject_area(subject_position - subject_center, subject_size);
        IntRect object_area(object_position - object_center, object_size);

        auto intersect_area = intersection(subject_area, object_area);
        if (intersect_area.width >= 0 && intersect_area.height >= 0)
        {


          // We know that either the subject or object's alignment is 0.
          // If it's the object, have it switch places with the object to make
          // sure we meet the requirements of the algorithm.
          if (intersect_area.left == object_area.left)
          {
            return test_collision_impl(object, subject, object_position, subject_position, 
                                       object_center, subject_center, intersect_area);
          }

          else
          {
            return test_collision_impl(subject, object, subject_position, object_position, 
                                       subject_center, object_center, intersect_area);
          }
        }

        return CollisionPoint();
      }
    }


    CollisionPoint test_scenery_collision(const CollisionMaskFrame& subject, const CollisionMaskFrame& scenery,
                                          Vector2i subject_position)
    {
      auto subject_size = make_vector2(subject.row_width(), subject.row_count());
      auto subject_center = vector2_cast<std::int32_t>(subject_size / std::size_t(2));
      auto scenery_size = make_vector2(scenery.row_width(), scenery.row_count());

      return detail::test_collision(subject, scenery, 
                                    vector2_cast<std::int32_t>(subject_size),
                                    vector2_cast<std::int32_t>(scenery_size),
                                    subject_position, Vector2i(0, 0),
                                    subject_center, Vector2i(0, 0));
    }

    CollisionPoint test_collision(const CollisionMaskFrame& subject, const CollisionMaskFrame& object,
                                  Vector2i subject_position, Vector2i object_position)
    {
      auto subject_size = make_vector2(subject.row_width(), subject.row_count());
      auto subject_center = vector2_cast<std::int32_t>(subject_size / std::size_t(2));
      auto object_size = make_vector2(object.row_width(), object.row_count());
      auto object_center = vector2_cast<std::int32_t>(object_size / std::size_t(2));

      return detail::test_collision(subject, object,
                                    vector2_cast<std::int32_t>(subject_size),
                                    vector2_cast<std::int32_t>(object_size),
                                    subject_position, object_position,
                                    subject_center, object_center);
    }
  }
}
