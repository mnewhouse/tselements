/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "catch.hpp"

#include "resources/track_loader.hpp"
#include "resources/track.hpp"
#include "resources/tile_library.hpp"
#include "resources/texture_library.hpp"
#include "resources/track_layer.hpp"
#include "resources/pattern_builder.hpp"

#include "scene/track_scene_generator_detail.hpp"

#include "graphics/image.hpp"

#include <iostream>

using namespace ts;

TEST_CASE("Track loading is a complex process, which is required to work perfectly in order for the game to be playable")
{
  resources::TrackLoader track_loader;
  REQUIRE_THROWS(track_loader.load_from_file("assets/tracks/doesnotexist.trk"));
  REQUIRE_NOTHROW(track_loader.load_from_file("assets/tracks/test.trk"));

  auto track = track_loader.get_result();
  REQUIRE(track.size() == Vector2u(1799, 1153));
  REQUIRE(track.author() == "Jovic");
  
  const auto& tile_library = track.tile_library();
  const auto& tiles = tile_library.tiles();

  const auto& tile_groups = tile_library.tile_groups();
  auto big_sand = tile_groups.find(7);
  REQUIRE(big_sand != tile_groups.end());

  if (big_sand != tile_groups.end())
  {
    REQUIRE(big_sand->id == 7);
    REQUIRE(big_sand->sub_tiles.size() == 16);
    
    if (big_sand->sub_tiles.size() == 16)
    {
      REQUIRE(big_sand->sub_tiles[5].id == 601);
      REQUIRE(big_sand->sub_tiles[5].position == Vector2i(59, 59));
      REQUIRE(big_sand->sub_tiles[5].rotation == 180);

      REQUIRE(big_sand->sub_tiles[15].id == 601);
      REQUIRE(big_sand->sub_tiles[15].position == Vector2i(178, 181));
      REQUIRE(big_sand->sub_tiles[15].rotation == 0);
    }
  }

  REQUIRE(track.layer_count() == 1);

  auto& layers = track.layers();
  REQUIRE(layers.size() == 1);

  if (layers.size() == 1)
  {
    auto& layer = layers[0];

    REQUIRE(layer.tiles.size() == 2);
    if (layer.tiles.size() == 2)
    {
      REQUIRE(layer.tiles[0].id == 25);
      REQUIRE(layer.tiles[0].position == Vector2i(523, 117));
      REQUIRE(layer.tiles[0].rotation == 126);
    }

    REQUIRE(layer.geometry.size() == 1);
    if (layer.geometry.size() == 1)
    {
      auto& v_array = layer.geometry[0];

      REQUIRE(v_array.vertices.size() == 3);      
      REQUIRE(v_array.texture_id == 10);

      if (v_array.vertices.size() == 4)
      {
        REQUIRE(vector2_round<std::int32_t>(v_array.vertices[0].position) == Vector2i(93, 125));
        REQUIRE(vector2_round<std::int32_t>(v_array.vertices[1].position) == Vector2i(177, 62));
        REQUIRE(vector2_round<std::int32_t>(v_array.vertices[2].position) == Vector2i(126, 163));
      }
    }
  }

  const auto& texture_library = track.texture_library();
  const auto& textures = texture_library.textures();

  SECTION("Track scene generator")
  {
    using namespace scene;
    auto image_mapping = detail::generate_image_mapping(track);
    auto placement_map = detail::generate_atlas_placement_map(track, image_mapping, { 2048, 2048 }, true);
    auto texture_map = detail::generate_resource_texture_map(track, placement_map);

    for (const auto& tile_def : tiles)
    {
      REQUIRE(detail::texture_rect_exists(placement_map, tile_def.image_file, tile_def.image_rect));

      auto range = texture_map.find(texture_map.tile_id(tile_def.id));
      REQUIRE(range.begin() != range.end());
    }

    for (const auto& texture : textures)
    {
      REQUIRE(detail::texture_rect_exists(placement_map, texture.file_name, texture.rect));

      auto range = texture_map.find(texture_map.texture_id(texture.id));
      REQUIRE(range.begin() != range.end());
    }

    detail::ImageLoader image_loader;
    std::size_t id = 0;
    for (const auto& atlas : placement_map.atlases)
    {
      auto surface = detail::build_atlas_surface(atlas, image_loader);
      graphics::save_image(surface, "assets/output/atlas_" + std::to_string(++id) + ".png");
    } 
  }

  SECTION("Pattern builder")
  {
    resources::TrackLoader banaring_loader;
    banaring_loader.load_from_file("assets/tracks/banaring.trk");

    auto banaring = banaring_loader.get_result();
    auto banaring_pattern = build_track_pattern(banaring);

    resources::save_pattern(banaring_pattern, "assets/output/banaring-pat.generated.png",
                            banaring.terrain_library());
  }
}

