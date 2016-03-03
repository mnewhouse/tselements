/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "catch.hpp"

#include "resources/pattern.hpp"

#include "resources/collision_mask.hpp"
#include "resources/collision_mask_detail.hpp"

#include "graphics/image.hpp"

using namespace ts;

TEST_CASE("Collision mask")
{
  auto pattern = resources::load_pattern("assets/cars/test-pat.png");
  REQUIRE(pattern.size().x == 32);
  REQUIRE(pattern.size().y == 32);

  auto frame_count = 64;
  resources::CollisionMask mask(resources::dynamic_mask, pattern, frame_count, [](auto p) { return p != 0; });

  sf::Image image;
  image.create(32 * frame_count, 32, sf::Color::Black);

  for (auto frame_id = 0; frame_id != frame_count; ++frame_id)
  {
    auto frame = mask.frame(frame_id);
    for (std::uint32_t y = 0; y != 32; ++y)
    {
      for (std::uint32_t x = 0; x != 32; ++x)
      {
        if (frame(x, y))
        {
          image.setPixel(frame_id * 32 + x, y, sf::Color::Red);
        }
      }
    }
  }

  image.saveToFile("assets/output/test-mask.png");

  {
    resources::Pattern object_pattern(Vector2u(32, 16));
    object_pattern(16, 0) = 1;
    object_pattern(20, 1) = 1;
    object_pattern(16, 2) = 1;

    resources::Pattern scenery_pattern(Vector2u(64, 64));
    scenery_pattern(35, 0) = 1;
    scenery_pattern(43, 1) = 1;
    scenery_pattern(16, 2) = 1;

    auto object_wall_test = [](auto p) { return p != 0; };
    auto scenery_wall_test = [](auto p, auto level) { return p != 0; };

    resources::CollisionMask object_mask(resources::dynamic_mask, object_pattern, 64, object_wall_test);
    resources::CollisionMask scenery_mask(scenery_pattern, 1, scenery_wall_test);

    auto object_frame = object_mask.frame(0);
    auto scenery_frame = scenery_mask.frame(0);

    auto collision = test_scenery_collision(object_frame, scenery_frame, Vector2i(35, 8));
    REQUIRE(collision.collided);

    collision = test_scenery_collision(object_frame, scenery_frame, Vector2i(39, 8));
    REQUIRE(collision.collided);

    collision = test_scenery_collision(object_frame, scenery_frame, Vector2i(16, 8));
    REQUIRE(collision.collided);
  }
}