/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resources/geometry.hpp"

#include "utility/rect.hpp"
#include "utility/color.hpp"
#include "utility/vector2.hpp"

#include <boost/optional.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    class Track;
    struct Tile;
    struct TileDefinition;
    struct TrackVertex;
    struct PlacedTile;
  }

  namespace scene
  {
    class TrackScene;
    class TextureMapping;

    // Takes a tile and its definition, generate exactly four vertices of type resource::Vertex
    // and write these to the given output iterator.
    std::array<resources::Vertex, 4>
      generate_tile_vertices(const resources::Tile& tile, const resources::TileDefinition& tile_def,
                             const IntRect& texture_rect, Vector2i fragment_offset, Vector2f texture_scale);

    std::array<resources::Face, 2> generate_tile_faces(std::uint32_t base_index);

    // This function takes a track and generates the vertices required to display it.
    // Writes the result to the TrackScene object. if use_relative_texture_coords is true,
    // divides the absolute texture coords by the texture size so that we get relative texture coords.    
    void build_track_vertices(const resources::Track& track, TrackScene& track_scene);
  }
}
