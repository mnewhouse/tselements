/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef COLLISION_MASK_HPP_5891819283
#define COLLISION_MASK_HPP_5891819283

#include <boost/optional.hpp>

#include "utility/rect.hpp"
#include "utility/vector2.hpp"
#include "utility/rotation.hpp"

#include <vector>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    class Pattern;

    static const struct dynamic_mask_t{} dynamic_mask;

    // The CollisionMask class turns a pattern map into a collision bitmap. It stores
    // the pixels in a space-efficient manner, using only one bit for each one.
    // Thanks to bitwise operations, this also makes it a time-efficient solution.
    class CollisionMask
    {
    public:
      template <typename WallTest>
      CollisionMask(const resources::Pattern& pattern, std::uint32_t level_count, WallTest&& wall_test);

      template <typename WallTest>
      CollisionMask(const resources::Pattern& pattern, IntRect rect, 
                    std::uint32_t level_count, WallTest&& wall_test);

      template <typename WallTest>
      CollisionMask(dynamic_mask_t, const resources::Pattern& pattern,
                    std::uint32_t frame_count, WallTest&& wall_test);

      template <typename WallTest>
      CollisionMask(dynamic_mask_t, const resources::Pattern& pattern, IntRect rect,
                    std::uint32_t frame_count, WallTest&& wall_test);

      using bitmask_type = std::uint32_t;

      struct FrameInterface
      {
      public:
        bool operator()(std::uint32_t x, std::uint32_t y) const;

        const bitmask_type* row_begin(std::uint32_t y) const;
        std::size_t row_width() const;
        std::size_t row_count() const;
        
      private:
        friend CollisionMask;
        FrameInterface(const bitmask_type* frame_bits, std::size_t row_width, std::size_t row_count);

        const bitmask_type* frame_bits_;
        std::size_t row_width_;
        std::size_t row_count_;
      };

      FrameInterface frame(std::uint32_t frame_id) const;
      FrameInterface rotation_frame(Rotation<double> rotation) const;

    private:     
      std::uint32_t row_width_;
      std::uint32_t frame_count_;
      Vector2u bitmap_size_;
      std::vector<bitmask_type> pixel_data_;
      double rotation_multiplier_;
    };

    using CollisionMaskFrame = CollisionMask::FrameInterface;

    struct CollisionPoint
    {
      CollisionPoint() = default;
      explicit CollisionPoint(Vector2i point_)
        : collided(true), point(point_)
      {}

      explicit operator bool() const { return collided; }

      bool collided = false;
      Vector2i point;
    };

    CollisionPoint test_collision(const CollisionMaskFrame& subject, const CollisionMaskFrame& object,
                                  Vector2i subject_position, Vector2i object_position);

    CollisionPoint test_scenery_collision(const CollisionMaskFrame& subject, const CollisionMaskFrame& scenery,
                                          Vector2i subject_position);
                                  
  }
}


#endif