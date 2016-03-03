/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "catch.hpp"

#include "utility/texture_atlas.hpp"

#include <vector>
#include <iostream>

using namespace ts;
using utility::TextureAtlas;

TEST_CASE("Let's see if the texture atlas works and packs in an efficient manner.")
{
  TextureAtlas atlas;  

  REQUIRE_FALSE(atlas.insert({ 100, 100 }));

  atlas = TextureAtlas({ 512, 512 });
  atlas.set_padding(0);

  REQUIRE(atlas.insert({ 512, 256 }));

  // No room for this one
  REQUIRE_FALSE(atlas.insert({ 512, 300 }));

  std::vector<IntRect> rects;
  for (int i = 0; i != 20; ++i)
  {
    auto rect = atlas.insert({ 50, 128 });
    REQUIRE(rect);
    if (rect) rects.push_back(*rect);
  }

  REQUIRE(atlas.insert({ 12, 128 }));
  REQUIRE(atlas.insert({ 12, 128 }));

  // Make sure none of the rectangles we got intersect with each other.
  for (auto outer = rects.begin(); outer != rects.end(); ++outer)
  {
    for (auto inner = std::next(outer); inner != rects.end(); ++inner)
    {
      REQUIRE_FALSE(intersects(*inner, *outer));
    }
  }
}
