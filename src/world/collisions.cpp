/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "collisions.hpp"
#include "entity.hpp"

#include "resources/collision_mask_detail.hpp"

#include <array>

namespace ts
{
  namespace world
  {
    namespace detail
    {
      Vector2<double> find_normal(unsigned mask)
      {
        enum : unsigned
        {
          top_left = 1,
          top_center = 1 << 1,
          top_right = 1 << 2,
          center_left = 1 << 3,
          center_right = 1 << 4,
          bottom_left = 1 << 5,
          bottom_center = 1 << 6,
          bottom_right = 1 << 7
        };

        auto test = [=](unsigned v)
        {
          return (mask & v) == v;
        };

        if (test(top_left))
        {
          if (test(bottom_left)) return{ -1.0, 0.0 };
          if (test(bottom_center)) return{ -2.0, 1.0 };
          if (test(center_left)) return{ -1.0, 0.0 };          
          if (test(center_right)) return{ -2.0, -1.0 };
          
          return{ -1.0, 1.0 };
        }

        if (test(center_left))
        {
          if (test(top_center)) return{ -1.0, -1.0 };
          if (test(top_right)) return{ -1.0, -2.0 };
          if (test(center_right)) return{ 0.0, -1.0 };
          if (test(bottom_right)) return{ 1.0, -2.0 };
          if (test(bottom_center)) return{ -1.0, 1.0 };

          return{ -1.0, 0.0 };
        }

        if (test(bottom_left))
        {
          if (test(top_center)) return{ -2.0, -1.0 };
          if (test(top_right)) return{ -1.0, -1.0 };
          if (test(center_right)) return{ -1.0, -2.0 };
          if (test(bottom_right)) return{ 0.0, -1.0 };
          
          return{ -1.0, -1.0 };
        }

        if (test(top_center))
        {
          if (test(bottom_center)) return{ -1.0, 0.0 };
          if (test(bottom_right)) return{ -2.0, 1.0 };
          
          return{ -1.0, 0.0 };
        }

        if (test(top_right)) 
        {
          if (test(bottom_center)) return{ -2.0, -1.0 };

          return{ -1.0, 1.0 };
        }

        if (test(bottom_center))
        {
          return{ -1.0, -1.0 };
        }

        return{ -1.0, 0.0 };
      }

      auto create_normal_lookup_table()
      {
        std::array<Vector2<double>, 256> result;

        for (unsigned mask = 0; mask != 256; ++mask)
        {
          result[mask] = normalize(find_normal(mask));
        }

        return result;
      }

      const auto normal_lookup_table = create_normal_lookup_table();

      struct PixelOrder
      {
        std::uint8_t pixels[8];

        constexpr std::uint8_t operator[](std::size_t i) const
        {
          return pixels[i];
        }
      };

      constexpr PixelOrder flip_orientation(PixelOrder pixels)
      {
        return
        {
          pixels[0], pixels[3], pixels[5],
          pixels[1], pixels[6],
          pixels[2], pixels[4], pixels[7]
        };
      }

      constexpr PixelOrder flip_horizontally(PixelOrder pixels)
      {
        return
        {
          pixels[2], pixels[1], pixels[0],
          pixels[4], pixels[3],
          pixels[7], pixels[6], pixels[5]
        };          
      }

      constexpr PixelOrder flip_vertically(PixelOrder pixels)
      {
        return
        {
          pixels[5], pixels[6], pixels[7],
          pixels[3], pixels[4],
          pixels[0], pixels[1], pixels[2]
        };
      }

      inline unsigned create_pixel_mask(const bool(&pixels)[8], PixelOrder pixel_order)
      {
        unsigned result = 0;
        result |= pixels[pixel_order[0]] << 0;
        result |= pixels[pixel_order[1]] << 1;
        result |= pixels[pixel_order[2]] << 2;
        result |= pixels[pixel_order[3]] << 3;
        result |= pixels[pixel_order[4]] << 4;
        result |= pixels[pixel_order[5]] << 5;
        result |= pixels[pixel_order[6]] << 6;
        result |= pixels[pixel_order[7]] << 7;

        return result;
      }
    }

    static Vector2<double> compute_collision_normal(const CollisionMaskFrame& frame, Vector2i local_point,
                                                    Vector2<double> entry_vector)
    {
      auto direction = (entry_vector.x >= 0.0 ? 1 : 0) | (entry_vector.y >= 0.0 ? 2 : 0);
      direction |= (std::abs(entry_vector.y) > std::abs(entry_vector.x)) << 2;

      constexpr unsigned left_to_right = 1;
      constexpr unsigned right_to_left = 0;
      constexpr unsigned top_to_bottom = 2;
      constexpr unsigned bottom_to_top = 0;
      constexpr unsigned steep = 4;

      const auto left = local_point.x - 1;
      const auto top = local_point.y - 1;
      const auto right = local_point.x + 1;
      const auto bottom = local_point.y + 1;

      const auto left_valid = left >= 0;
      const auto top_valid = top >= 0;
      const auto right_valid = static_cast<std::size_t>(right) < frame.row_width();
      const auto bottom_valid = static_cast<std::size_t>(bottom) < frame.row_count();

      constexpr detail::PixelOrder pixel_order = { 0, 1, 2, 3, 4, 5, 6, 7 };

      const bool pixels[8] =
      {
        (left_valid & top_valid & frame(left, top)) != 0,
        (top_valid & frame(local_point.x, top)) != 0,
        (right_valid & top_valid & frame(right, top)) != 0,

        (left_valid & frame(left, local_point.y)) != 0,
        (right_valid & frame(right, local_point.y)) != 0,
        
        (left_valid & bottom_valid & frame(left, bottom)) != 0,
        (bottom_valid & frame(local_point.x, bottom)) != 0,
        (right_valid & bottom_valid & frame(right, bottom)) != 0
      };
      
      switch (direction)
      {
      case left_to_right | top_to_bottom:
      {
        const auto pixel_mask = create_pixel_mask(pixels, pixel_order);

        return detail::normal_lookup_table[pixel_mask];
      }

      case left_to_right | bottom_to_top:
      {
        const auto pixel_mask = create_pixel_mask(pixels, flip_vertically(pixel_order));

        auto normal = detail::normal_lookup_table[pixel_mask];
        return{ normal.x, -normal.y };
      }

      case right_to_left | top_to_bottom:
      {
        const auto pixel_mask = create_pixel_mask(pixels, flip_horizontally(pixel_order));

        auto normal = detail::normal_lookup_table[pixel_mask];
        return{ -normal.x, normal.y };
      }

      case right_to_left | bottom_to_top:
      {
        const auto pixel_mask = create_pixel_mask(pixels, flip_vertically(flip_horizontally(pixel_order)));                

        auto normal = detail::normal_lookup_table[pixel_mask];
        return{ -normal.x, -normal.y };
      }

      case left_to_right | top_to_bottom | steep:
      {
        const auto pixel_mask = create_pixel_mask(pixels, flip_orientation(pixel_order));

        auto normal = detail::normal_lookup_table[pixel_mask];
        return{ normal.y, normal.x };
      }

      case left_to_right | bottom_to_top | steep:
      {
        const auto pixel_mask = create_pixel_mask(pixels, flip_orientation(flip_vertically(pixel_order)));

        auto normal = detail::normal_lookup_table[pixel_mask];
        return{ normal.y, -normal.x };
      }

      case right_to_left | top_to_bottom | steep:
      {
        const auto pixel_mask = create_pixel_mask(pixels, flip_orientation(flip_horizontally(pixel_order)));

        auto normal = detail::normal_lookup_table[pixel_mask];
        return{ -normal.y, normal.x };
      }

      case right_to_left | bottom_to_top | steep:
      default:
      {
        const auto pixel_mask = create_pixel_mask(pixels, flip_orientation(flip_horizontally(flip_vertically(pixel_order))));

        auto normal = detail::normal_lookup_table[pixel_mask];
        return{ -normal.y, -normal.x };
      }
      }
    }

    CollisionResult examine_scenery_collision(const CollisionMaskFrame& scenery, Vector2i global_point,
                                              Vector2<double> subject_velocity, Vector2<double> entry_vector,
                                              double bounce_factor)
    {
      CollisionResult result;
      result.point = global_point;
      result.normal = compute_collision_normal(scenery, global_point, entry_vector);
      result.impact = std::abs(dot_product(subject_velocity, result.normal));
      result.bounce_factor = bounce_factor;

      return result;
    }

    void resolve_scenery_collision(const CollisionResult& collision, Entity& entity,
                                   Rotation<double> rotation_delta)
    {
      // Set velocity and rotational velocity according to the parameters.

      auto velocity = entity.velocity();
      
      auto new_velocity = velocity - 2.0 * collision.normal *
        dot_product(collision.normal, velocity);

      new_velocity *= collision.bounce_factor;
      entity.set_velocity(new_velocity);
    }
  }
}