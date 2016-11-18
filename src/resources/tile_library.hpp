/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "tiles.hpp"
#include "resource_library_interface.hpp"

#include <boost/iterator.hpp>
#include <boost/utility/string_ref.hpp>

#include <set>
#include <unordered_set>
#include <string>

namespace ts
{
  namespace resources
  {
    class TileLibrary;

    namespace detail
    {
      // These comparator objects allow for tile definitions can be compared to their
      // associated tile ids, allowing for convenient lookup in the exposed tile library interfaces.
      struct TileDefinitionComparator
      {
        using is_transparent = std::true_type;
        bool operator()(const TileDefinition& a, const TileDefinition& b) const;
        bool operator()(const TileDefinition& a, TileId tile_id) const;
        bool operator()(TileId tile_id, const TileDefinition& a) const;
      };

      struct TileGroupDefinitionComparator
      {
        using is_transparent = std::true_type;
        bool operator()(const TileGroupDefinition& a, const TileGroupDefinition& b) const;
        bool operator()(const TileGroupDefinition& a, TileId tile_id) const;
        bool operator()(TileId tile_id, const TileGroupDefinition& a) const;
      };
    }

    // The TileLibrary class exposes two types of resources. First we have tile definitions, which 
    // define how a tile should be drawn and how it interacts with the game world. Additionally,
    // we have tile groups, which contain a set of placed sub-tiles.
    class TileLibrary
    {
    private:
      using tile_container_type = std::set<TileDefinition, detail::TileDefinitionComparator>;
      using tile_group_container_type = std::set<TileGroupDefinition, detail::TileGroupDefinitionComparator>;

    public:
      using tile_interface_type = ResourceLibraryInterface<tile_container_type, SetSearchPolicy>;
      using tile_group_interface_type = ResourceLibraryInterface<tile_group_container_type, SetSearchPolicy>;

      using tile_iterator = tile_interface_type::iterator;
      using tile_group_iterator = tile_group_interface_type::iterator;

      struct TileDefinitionInterface;

      // This function allows for efficiently defining larger sets of tiles, without making multiple copies
      // of the file name strings or other kinds of inefficiencies.
      TileDefinitionInterface define_tile_set(boost::string_ref pattern_file, boost::string_ref image_file);

      tile_group_iterator define_tile_group(const TileGroupDefinition& tile_group_def);

      tile_interface_type tiles() const;
      tile_group_interface_type tile_groups() const;     

    private:
      tile_iterator define_tile(const TileDefinition& tile_def);

      tile_container_type tile_definitions_;
      tile_group_container_type tile_group_definitions_;

      std::unordered_set<std::string> filename_strings_;
    };

    struct TileLibrary::TileDefinitionInterface
    {
      tile_iterator define_tile(TileId tile_id, IntRect pattern_rect, IntRect image_rect);

    private:
      friend TileLibrary;
      TileDefinitionInterface(TileLibrary* tile_library, boost::string_ref pattern_file, boost::string_ref image_file);

      TileLibrary* tile_library_;
      boost::string_ref pattern_file_;
      boost::string_ref image_file_;
    };
  }
}
