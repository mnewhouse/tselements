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
#include <boost/geometry/index/rtree.hpp>

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
        const resources::Pattern* mask = nullptr;
        
        IntRect pattern_rect; 
        IntRect mask_rect;

        Vector2i position;
        std::array<std::int32_t, 2> transformation;

        std::uint8_t alpha;
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
      boost::variant<map_components::Pattern, map_components::Face, map_components::Base> data;
      std::uint32_t level;
      std::uint32_t z_index;
    };

    class TerrainMap
    {
    public:
      template <typename MapComponentRange>
      explicit TerrainMap(const MapComponentRange& components, resources::PatternStore pattern_store);

      resources::TerrainDefinition terrain_at(Vector2i position, std::int32_t level, 
                                              const resources::TerrainLibrary& terrain_lib) const;

    private:
      using point_type = boost::geometry::model::point<std::int32_t, 2, boost::geometry::cs::cartesian>;
      using box_type = boost::geometry::model::box<point_type>;
      using value_type = std::pair<box_type, TerrainMapComponent>;
      using tree_type = boost::geometry::index::rtree<value_type, boost::geometry::index::quadratic<32>>;

      static box_type bounding_box(const TerrainMapComponent& component);
      static box_type bounding_box(const map_components::Pattern& pattern);
      static box_type bounding_box(const map_components::Face& face);
      static box_type bounding_box(const map_components::Base& base);

      std::vector<tree_type> component_maps_;      

      struct InternalTerrainDescriptor
        : TerrainDescriptor
      {
        std::uint32_t z_index;
      };
      mutable std::vector<InternalTerrainDescriptor> terrain_buffer_;

      resources::PatternStore pattern_store_;
    };
  }
}

#include "terrain_map.inl"