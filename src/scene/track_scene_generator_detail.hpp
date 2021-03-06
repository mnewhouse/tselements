/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "texture_mapping.hpp"

#include "utility/string_utilities.hpp"
#include "utility/rect.hpp"
#include "utility/vector2.hpp"

#include "graphics/image_loader.hpp"
#include "graphics/image.hpp"

#include <unordered_map>
#include <vector>

namespace ts
{
  namespace resources
  {
    class Track;
  }

  namespace scene
  {
    class TrackScene;

    namespace detail
    {
      struct AtlasPlacement
      {
        IntRect atlas_rect;
        IntRect source_rect;
        IntRect full_source_rect;
      };

      struct AtlasFragment
        : AtlasPlacement
      {
        std::size_t atlas_id;
        boost::string_ref image_file;
      };

      struct AtlasDefinition
      {
        Vector2i size;
        std::unordered_map<boost::string_ref, std::vector<AtlasPlacement>, StringRefHasher> image_data;
      };

      struct PlacementMap
      {
        std::vector<AtlasDefinition> atlases;
        std::vector<AtlasFragment> atlas_fragments;
      };

      using ImageMapping = std::unordered_map<boost::string_ref, std::vector<IntRect>, StringRefHasher>;
      ImageMapping generate_image_mapping(const resources::Track& track);

      TextureMapping generate_resource_texture_map(const resources::Track& track,
                                                   const PlacementMap& placement_map,
                                                   std::vector<std::unique_ptr<graphics::Texture>> atlas_textures);
                                                   

      PlacementMap generate_atlas_placement_map(const resources::Track& track, const ImageMapping& image_mapping,
                                                Vector2i atlas_size, bool include_all_assets = false);

      bool texture_rect_exists(const AtlasDefinition& atlas, boost::string_ref file_name, const IntRect& rect);
      bool texture_rect_exists(const PlacementMap& placement_map, boost::string_ref file_name, const IntRect& rect);

      IntRect find_enclosing_rect(const ImageMapping& image_mapping,
                                  boost::string_ref file_name, const IntRect& rect);

      TrackScene generate_track_scene(const resources::Track& track, const PlacementMap& placement_map, bool all_assets);

      using ImageLoader = graphics::DefaultImageLoader;
      sf::Image build_atlas_image(const AtlasDefinition &atlas, ImageLoader& image_loader);
    }
  }
}
