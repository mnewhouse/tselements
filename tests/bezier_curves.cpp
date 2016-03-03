/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "catch.hpp"

#include "utility/bezier_path.hpp"
#include "utility/bezier_path_detail.hpp"

#include "graphics/image.hpp"

#include <SFML/Graphics/Image.hpp>

#include <iostream>

using namespace ts;

TEST_CASE("Bezier curves")
{
  using utility::BezierPath;
  using utility::BezierCurve;
  BezierPath path;
  path.push_back({ { 5.0, 5.0 }, { 45.0, 5.0 }, { 80.0, 40.0 }, { 80.0, 80.0 } }); // Insert in that order
  //path.push_back({ {}, { 30.0, 30.0 }, { 50.0, 30.0 }, { 60.0, 35.0 } }); // Ignore first value
  REQUIRE(path.size() == 1);

  sf::Image image;
  image.create(200, 200, sf::Color::Black);

  auto draw_path = [&image](const auto& path, sf::Color color)
  {
    for (BezierCurve curve : path)
    {
      for (auto i = 0; i <= 100; ++i)
      {
        auto point = vector2_cast<std::int32_t>(bezier_point_at(curve, 0.01 * i));
        image.setPixel(point.x + 100, point.y + 100, color);        
      }
    }
  };

  draw_path(path, sf::Color::Red);
}