/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "tile_library.hpp"

#include "utility/debug_log.hpp"

namespace ts
{
  namespace resources
  {
    namespace detail
    {
      bool TileDefinitionComparator::operator()(const TileDefinition& a, const TileDefinition& b) const
      {
        return a.id < b.id;
      }

      bool TileDefinitionComparator::operator()(const TileDefinition& tile_def, TileId tile_id) const
      {
        return tile_def.id < tile_id;
      }

      bool TileDefinitionComparator::operator()(TileId tile_id, const TileDefinition& tile_def) const
      {
        return tile_id < tile_def.id;
      }

      bool TileGroupDefinitionComparator::operator()(const TileGroupDefinition& a, const TileGroupDefinition& b) const
      {
        return a.id < b.id;
      }

      bool TileGroupDefinitionComparator::operator()(const TileGroupDefinition& tile_group_def, TileId tile_id) const
      {
        return tile_group_def.id < tile_id;
      }

      bool TileGroupDefinitionComparator::operator()(TileId tile_id, const TileGroupDefinition& tile_group_def) const
      {
        return tile_id < tile_group_def.id;
      }
    }

    TileLibrary::TileDefinitionInterface TileLibrary::define_tile_set(boost::string_ref pattern_file, boost::string_ref image_file)
    {
      // Store the strings for internal use -- the container will make sure the pointers are stable.
      std::string pattern_string(pattern_file.data(), pattern_file.size());
      std::string image_string(image_file.data(), image_file.size());

      auto pattern_it = filename_strings_.insert(std::move(pattern_string)).first;
      auto image_it = filename_strings_.insert(std::move(image_string)).first;

      pattern_file = { pattern_it->data(), pattern_it->size() };
      image_file = { image_it->data(), image_it->size() };

      // And return an interface to define all the tiles in this set.
      return TileDefinitionInterface(this, pattern_file, image_file);
    }

    TileLibrary::tile_iterator TileLibrary::define_tile(const TileDefinition& tile_def)
    {
      auto result = tile_definitions_.insert(tile_def);
      if (result.second)
      {
        DEBUG_AUXILIARY << "Info: tile definition added. [tile_id=" << tile_def.id << "]" << debug::endl;
      }

      else
      {
        tile_definitions_.erase(tile_def);
        result = tile_definitions_.insert(tile_def);
        DEBUG_RELEVANT << "Warning: tile definition already exists, overwriting. [tile_id=" << tile_def.id << "]" << debug::endl;
      }

      return result.first;
    }

    TileLibrary::tile_group_iterator TileLibrary::define_tile_group(const TileGroupDefinition& tile_group_def)
    {
      auto result = tile_group_definitions_.insert(tile_group_def);
      if (result.second)
      {
        DEBUG_AUXILIARY << "Info: tile group definition added. [group_id=" << tile_group_def.id << ", group_size=" <<
          tile_group_def.sub_tiles.size() << "]" << debug::endl;
      }

      else
      {
        tile_group_definitions_.erase(tile_group_def);
        result = tile_group_definitions_.insert(tile_group_def);

        DEBUG_RELEVANT << "Warning: tile group definition already exists, overwriting. [group_id = " <<
          tile_group_def.id << ", group_size = " << tile_group_def.sub_tiles.size() << "]" << debug::endl;
      }


      return result.first;
    }
    
    void TileLibrary::define_collision_shape(TileId tile_id, CollisionShape collision_shape)
    {
      auto tile_it = tile_definitions_.find(tile_id);
      if (tile_it != tile_definitions_.end())
      {        
        // Bit of an ugly hack, but this should be fine because the collision shape doesn't affect the ordering.
        const_cast<CollisionShape&>(tile_it->collision_shape) = std::move(collision_shape);
      }
    }

    TileLibrary::tile_interface_type TileLibrary::tiles() const
    {
      return tile_interface_type(&tile_definitions_);
    }

    TileLibrary::tile_group_interface_type TileLibrary::tile_groups() const
    {
      return tile_group_interface_type(&tile_group_definitions_);
    }

    TileLibrary::TileDefinitionInterface::TileDefinitionInterface(TileLibrary* tile_library,
                                                                  boost::string_ref pattern_file, boost::string_ref image_file)
      : tile_library_(tile_library),
        pattern_file_(pattern_file),
        image_file_(image_file)
    {
    }

    TileLibrary::tile_iterator TileLibrary::TileDefinitionInterface::define_tile(TileId tile_id, 
                                                                                 IntRect pattern_rect, IntRect image_rect)
    {
      TileDefinition tile_def;
      tile_def.id = tile_id;
      tile_def.pattern_file = pattern_file_;
      tile_def.image_file = image_file_;
      tile_def.pattern_rect = pattern_rect;
      tile_def.image_rect = image_rect;
      
      return tile_library_->define_tile(tile_def);
    }
  }
}