/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "elevation_map_3d.hpp"
#include "texture_library_3d.hpp"
#include "track_path_3d.hpp"

#include <boost/range/iterator_range.hpp>

#include <vector>
#include <map>
#include <cstdint>

namespace ts
{
  namespace resources3d
  {
    class Track
    {
    public:
      explicit Track(Vector2i size);
      explicit Track(Vector2i size, ElevationMap elevation_map);

      TextureLibrary& texture_library();
      const TextureLibrary& texture_library() const;

      const ElevationMap& elevation_map() const;
      void assign_elevation_map(ElevationMap map);

      Vector2i size() const;
      void resize(Vector2i new_size);

      PathLayer* create_path_layer();
      
      boost::iterator_range<const PathLayer* const*> path_layers() const;
      

    private:
      Vector2i size_;

      ElevationMap elevation_map_;
      TextureLibrary texture_library_;

      std::map<std::uint32_t, PathLayer> path_layers_;
      std::vector<PathLayer*> path_layer_order_;
    };
  }
}