/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "catch.hpp"

#include "resources/car_loader.hpp"
#include "resources/car_store.hpp"

#include "utility/string_utilities.hpp"

#include <boost/utility/string_ref.hpp>

#include <functional>

using namespace ts;

TEST_CASE("Car definition loading tests.")
{
  SECTION("CarLoader")
  {
    resources::CarLoader car_loader;
    car_loader.load_cars_from_file("assets/cars/cardef.car");

    auto result = car_loader.get_result();
    REQUIRE(result.size() == 5);
    if (result.size() == 5)
    {
      const boost::string_ref image_paths[] =
      {
        "easysliderc.png", "sliderc.png", "speederc.png", "antisliderc.png", "spinnerc.png"
      };

      for (auto idx = 0; idx != 5; ++idx)
      {
        const auto& car_def = result[idx];
        boost::string_ref image_path = car_def.image_path;

        REQUIRE(image_path.ends_with(image_paths[idx]));
        REQUIRE(car_def.image_rect == IntRect(0, 0, 2048, 32));
        REQUIRE(car_def.image_type == resources::CarImage::Prerotated);
      }
    }
  }

  SECTION("CarStore")
  {
    resources::CarStore car_store;
    car_store.load_car_directory("assets/cars");

    const auto& car_interface = car_store.car_definitions();

    REQUIRE(car_interface.size() == 5);
    if (car_interface.size() == 5)
    {
      boost::string_ref car_names[] = { "antislider", "easyslider", "slider", "speeder", "spinner" };
      std::size_t idx = 0;

      for (auto car_def : car_interface)
      {
        primitive_tolower(car_def.car_name);
        REQUIRE(car_def.car_name == car_names[idx]);
        ++idx;
      }
    }
  }
}