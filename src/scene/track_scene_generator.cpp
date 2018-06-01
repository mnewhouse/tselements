/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "track_scene_generator.hpp"
#include "track_scene_generator_detail.hpp"

#include "utility/vector2.hpp"

#include <GL/glew.h>

#include <memory>
#include <cstdint>
#include <algorithm>
#include <stdexcept>

namespace ts
{
  namespace scene
  {
    static const std::int32_t desired_atlas_size = 2048;
    
    TrackScene generate_track_scene(const resources::Track& track, bool include_all_assets)
    {
      /* In order to generate a track scene, we must:
         * Generate one or more texture atlases so that the track can be rendered efficiently.
         * Load image files at most once, and keep them in the cache.
         * Create the texture images and once this is done, the textures themselves.
         * Finally, we have to generate the vertices that make up the track.
         */
      
      // The first thing we have to do is see which tiles we are working with, possibly
      // filter out the ones we don't need, and also make sure we only have a single
      // entry for tiles that overlap sufficiently.

      auto image_mapping = detail::generate_image_mapping(track);

      
      std::int32_t atlas_size = std::min(desired_atlas_size, graphics::max_texture_size());

      auto placement_map = detail::generate_atlas_placement_map(track, image_mapping,
                                                                make_vector2(atlas_size, atlas_size),
                                                                include_all_assets);

      return detail::generate_track_scene(track, placement_map, include_all_assets);
    }
  }
}