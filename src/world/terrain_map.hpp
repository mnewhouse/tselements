/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resources/terrain_definition.hpp"
#include "resources/pattern.hpp"
#include "resources/pattern_store.hpp"

#include "utility/vector2.hpp"
#include "utility/rect.hpp"

#include <boost/variant.hpp>

#include <utility>
#include <vector>
#include <cstdint>

namespace ts
{
  namespace resources
  {
    class TerrainLibrary;
  }

  namespace world
  {
    namespace map_components
    {
      struct Pattern
      {       
        const resources::Pattern* pattern;        
        
        Vector2i position;
        IntRect rect;
        
        std::array<std::int32_t, 2> transformation;   
      };

      struct Mask
      {
        const resources::Pattern* mask;
        Vector2i position;
        IntRect rect;
        std::array<std::int32_t, 2> transformation;

        std::uint8_t alpha;
        resources::TerrainId terrain_id;
      };

      struct Face
      {
        resources::TerrainId terrain_id;
        std::uint8_t alpha;

        std::array<Vector2i, 3> vertices;
        const resources::Pattern* mask = nullptr;

        IntRect mask_rect;
      };

      struct Base
      {
        resources::TerrainId terrain_id;
        std::uint8_t alpha = 255;
        IntRect rect;
      };
    }

    struct TerrainDescriptor
    {
      resources::TerrainId terrain_id;
      std::uint8_t alpha;
    };

    struct TerrainMapComponent
    {
      boost::variant<map_components::Pattern, map_components::Face> data;
      std::uint32_t level;
    };

    class TerrainMap
    {
    public:      
      explicit TerrainMap(std::vector<TerrainMapComponent> components, resources::PatternStore pattern_store, 
                          Vector2i track_size, resources::TerrainId base_terrain);

      resources::TerrainDefinition terrain_at(Vector2i position, std::int32_t level, 
                                              const resources::TerrainLibrary& terrain_lib) const;

    private:
      std::vector<TerrainMapComponent> terrain_components_;
      std::vector<std::uint32_t> component_mapping_;
      std::vector<std::pair<std::uint32_t, std::uint32_t>> terrain_cells_;

      std::int32_t cell_bits_ = 6;
      Vector2i track_size_;
      Vector2i num_cells_;
      resources::TerrainId base_terrain_ = 0;

      resources::PatternStore pattern_store_;
    };
  }
}
