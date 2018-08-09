/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "catch.hpp"

#include "resources/track_loader.hpp"
#include "resources/track.hpp"
#include "resources/tile_library.hpp"
#include "resources/texture_library.hpp"
#include "resources/track_layer.hpp"

#include "scene/track_scene_generator_detail.hpp"
#include "scene/track_scene.hpp"

#include "graphics/image.hpp"

#include <iostream>

using namespace ts;

TEST_CASE("Track loading is a complex process, which is required to work perfectly in order for the game to be playable")
{
  resources::TrackLoader track_loader;
  REQUIRE_THROWS(track_loader.load_from_file("assets/tracks/doesnotexist.trk"));
  REQUIRE_NOTHROW(track_loader.load_from_file("assets/tracks/test.trk"));

  auto track = track_loader.get_result();
  REQUIRE(track.size() == Vector2i(1799, 1153));
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
    auto layer_tiles = layer.tiles();
    REQUIRE(layer_tiles != nullptr);
    if (layer_tiles != nullptr)
    {
      auto& t = *layer_tiles;
      REQUIRE(t.size() == 2);
      if (t.size() == 2)
      {
        REQUIRE(t[0].id == 25);
        REQUIRE(t[0].position == Vector2i(523, 117));
        REQUIRE(t[0].rotation == 126);
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

    std::vector<std::unique_ptr<graphics::Texture>> dummy_textures(placement_map.atlases.size());
    auto texture_map = detail::generate_resource_texture_map(track, placement_map, std::move(dummy_textures));

    for (const auto& tile_def : tiles)
    {
      REQUIRE(detail::texture_rect_exists(placement_map, tile_def.image_file, tile_def.image_rect));

      auto range = texture_map.find(texture_map.tile_id(tile_def.id));
      REQUIRE(range.begin() != range.end());
    }

    {
      graphics::Texture a, b, c;

      resources::Vertex verts[4] = {};
      verts[0].color = Colorb(255, 0, 0, 255);
      verts[1].color = Colorb(255, 255, 0, 255);
      verts[2].color = Colorb(0, 255, 0, 255);
      verts[3].color = Colorb(0, 0, 255, 0);

      resources::Face faces[2];
      faces[0].indices = { 0, 1, 2 };
      faces[1].indices = { 1, 2, 3 };

      resources::TrackLayer track_layer(resources::TrackLayerType::Tiles, 0, "dummy");

      /*
      scene::TrackSceneLayer layer(&track_layer);
      layer.append_item_geometry(0, &a, verts, 4, faces, 2, 0);
      layer.append_item_geometry(1, &a, verts, 4, faces, 2, 0);
      layer.append_item_geometry(1, &a, verts, 4, faces, 2, 0);
      REQUIRE(layer.components().size() == 1);

      layer.append_item_geometry(1, &a, verts, 4, faces, 2, 1);
      REQUIRE(layer.components().size() == 2);
      REQUIRE(layer.vertices().size() == 16);
      REQUIRE(layer.faces().size() == 8);

      layer.remove_item_geometry(1);
      REQUIRE(layer.components().size() == 1);
      REQUIRE(layer.vertices().size() == 4);
      REQUIRE(layer.faces().size() == 2);
      layer.append_item_geometry(1, &b, verts, 4, faces, 2, 0);
      REQUIRE(layer.components().size() == 2);
      REQUIRE(layer.vertices().size() == 8);
      REQUIRE(layer.faces().size() == 4);
      */
    }

    detail::ImageLoader image_loader;
    std::size_t id = 0;
    for (const auto& atlas : placement_map.atlases)
    {
      auto surface = detail::build_atlas_image(atlas, image_loader);
      graphics::save_image(surface, "assets/output/atlas_" + std::to_string(++id) + ".png");    
    }
  }
}

