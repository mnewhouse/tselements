/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "track_scene_generator_detail.hpp"
#include "track_scene.hpp"
#include "track_vertices.hpp"

#include "utility/texture_atlas.hpp"

#include "resources/track.hpp"
#include "resources/track_layer.hpp"
#include "resources/tile_library.hpp"
#include "resources/texture_library.hpp"

#include "graphics/image_loader.hpp"
#include "graphics/image.hpp"
#include "graphics/texture.hpp"
#include "graphics/gl_check.hpp"

#include <algorithm>

namespace ts
{
  namespace scene
  {
    namespace detail
    {
      template <std::size_t MaxId>
      struct AssetCache
      {
      public:
        explicit AssetCache()
          : asset_cache_(MaxId * 8, false)
        {
        }

        void set(std::size_t atlas_id, std::size_t asset_id)
        {
          asset_cache_[atlas_id * MaxId + asset_id] = true;
        }

        bool is_set(std::size_t atlas_id, std::size_t asset_id) const
        {
          return asset_cache_[atlas_id * MaxId + asset_id];
        }

        void ensure_size(std::size_t atlas_id)
        {
          auto needed_size = atlas_id * MaxId;
          if (asset_cache_.size() < needed_size)
          {
            asset_cache_.resize(std::max(asset_cache_.size(), needed_size), false);
          }
        }

      private:
        std::vector<bool> asset_cache_;
      };

      // This function merges the sufficiently overlapping sub-rectangles into one, in order
      // to save space in the final texture atlas.
      template <typename BidirIt>
      static BidirIt combine_overlapping_rectangles(BidirIt rect_it, BidirIt rect_end)
      {
        while (rect_it != rect_end)
        {
          auto& rect = *rect_it;
          auto intersect_it = std::partition(std::next(rect_it), rect_end,
                                             [rect](const auto& other)
          {
            return !intersects(rect, other);
          });

          bool merged = false;

          // For all rectangles that intersect, test if they intersect sufficiently.
          // If they do, combine them.
          auto intersects_sufficiently = [rect](const auto& other_rect)
          {
            auto combination = combine(rect, other_rect);
            auto total_area = rect.width * rect.height + other_rect.width * other_rect.height;

            return total_area >= combination.width * combination.height;
          };

          for (auto it = std::find_if(intersect_it, rect_end, intersects_sufficiently); it != rect_end;)
          {
            merged = true;
            rect = combine(rect, *it);
            --rect_end;

            if (it != rect_end)
            {
              std::iter_swap(it, rect_end);
              it = std::find_if(std::next(it), rect_end, intersects_sufficiently);
            }
          }

          if (!merged)
          {
            ++rect_it;
          }
        }

        return rect_end;
      }

      
      ImageMapping generate_image_mapping(const resources::Track& track)
      {
        ImageMapping image_mapping;

        const auto& tile_interface = track.tile_library().tiles();
        const auto& texture_interface = track.texture_library().textures();
        
        // Store all the tile and texture rectangles present in the track...
        for (auto tile_def : tile_interface)
        {
          image_mapping[tile_def.image_file].push_back(tile_def.image_rect);
        }

        for (auto texture_def : texture_interface)
        {
          image_mapping[texture_def.file_name].push_back(texture_def.rect);
        }

        // Then combine the overlapping ones.
        for (auto& image_info : image_mapping)
        {
          auto& rects = image_info.second;
          rects.erase(detail::combine_overlapping_rectangles(rects.begin(), rects.end()), rects.end());
        }


        return image_mapping;
      }

      static IntRect find_enclosing_rect(const ImageMapping& image_mapping,
                                         boost::string_ref file_name, const IntRect& rect)
      {
        auto map_it = image_mapping.find(file_name);
        if (map_it != image_mapping.end())
        {
          const auto& rects = map_it->second;
          auto it = std::find_if(rects.begin(), rects.end(),
                                 [=](const IntRect& other)
          {
            return contains(other, rect);
          });

          if (it != rects.end())
          {
            return *it;
          }
        }

        return rect;
      }

      static void ensure_placement_map_size(PlacementMap& placement_map, std::size_t atlas_id)
      {
        if (atlas_id >= placement_map.atlases.size())
        {
          placement_map.atlases.resize(atlas_id + 1);
        }
      }

      bool texture_rect_exists(const AtlasDefinition& atlas, boost::string_ref file_name, const IntRect& rect)
      {
        auto map_it = atlas.image_data.find(file_name);
        if (map_it != atlas.image_data.end())
        {
          const auto& rects = map_it->second;
          return std::find_if(rects.begin(), rects.end(),
                              [=](const AtlasPlacement& placement)
          {
            return contains(placement.full_source_rect, rect);
          }) != rects.end();
        }

        return false;
      }

      bool texture_rect_exists(const PlacementMap& placement_map, boost::string_ref file_name, const IntRect& rect)
      {
        return std::find_if(placement_map.atlases.begin(), placement_map.atlases.end(),
                            [=](const AtlasDefinition& atlas)
        {
          return texture_rect_exists(atlas, file_name, rect);
        }) != placement_map.atlases.end();
      }

      bool fragmented_texture_rect_exists(const PlacementMap& placement_map, boost::string_ref file_name, 
                                          const IntRect& rect)
      {
        return std::find_if(placement_map.atlas_fragments.begin(), placement_map.atlas_fragments.end(),
                            [=](const AtlasFragment& fragment)
        {
          return file_name == fragment.image_file && contains(fragment.full_source_rect, rect);
        }) != placement_map.atlas_fragments.end();
      }

      void add_atlas_placement(AtlasDefinition& atlas, boost::string_ref file_name, const IntRect& atlas_rect,
                               const IntRect& source_rect, const IntRect& full_source_rect)
      {
        auto& placement_list = atlas.image_data[file_name];

        AtlasPlacement atlas_placement;
        atlas_placement.atlas_rect = atlas_rect;
        atlas_placement.source_rect = source_rect;
        atlas_placement.full_source_rect = full_source_rect;

        placement_list.push_back(atlas_placement);
      }

      sf::Image build_atlas_surface(const AtlasDefinition& atlas, ImageLoader& image_loader)
      {
        sf::Image surface;
        surface.create(atlas.size.x, atlas.size.y, sf::Color(0, 0, 0, 0));

        // Copy the loaded images into the newly created atlas image.
        for (const auto& image_data : atlas.image_data)
        {
          const auto& image_file = image_data.first;
          const auto& placement_list = image_data.second;

          const auto& image = image_loader.load_image(image_file);
          for (const auto& placement : placement_list)
          {
            Vector2i position(placement.atlas_rect.left, placement.atlas_rect.top);
            auto src_rect = placement.source_rect;

            surface.copy(image, position.x, position.y, { src_rect.left, src_rect.top, src_rect.width, src_rect.height });
          }
        }

        return surface;
      }

      TextureMapping generate_resource_texture_map(const resources::Track& track, const PlacementMap& placement_map,
                                                   const TextureMapping::texture_type* atlas_textures)
      {
        TextureMapping texture_map;
        {
          // Need an enclosing scope to ensure the map_interface is destroyed before the return statement.
          auto map_interface = texture_map.create_mapping_interface();

          const auto& tile_library = track.tile_library();
          const auto& texture_library = track.texture_library();

          std::vector<AtlasPlacement> placement_buffer;
          auto map_resource_textures = [&](std::size_t resource_id, boost::string_ref image_file, const IntRect& image_rect)
          {
            std::size_t atlas_id = 0;
            for (const auto& atlas : placement_map.atlases)
            {
              auto image_it = atlas.image_data.find(image_file);
              if (image_it != atlas.image_data.end())
              {
                // Find all the existing atlas rects that already contain the image,
                // and store information for each one.

                placement_buffer.clear();
                std::copy_if(image_it->second.begin(), image_it->second.end(), std::back_inserter(placement_buffer),
                             [=](const AtlasPlacement& placement)
                {
                  return contains(placement.full_source_rect, image_rect);
                });

                for (const auto& placement : placement_buffer)
                {
                  // The rect we need may be only a sub-rect of the atlas rect we found. Take that into account.
                  IntRect partial_rect;
                  partial_rect.left = placement.atlas_rect.left + image_rect.left - placement.source_rect.left;
                  partial_rect.top = placement.atlas_rect.top + image_rect.top - placement.source_rect.top;
                  partial_rect.width = image_rect.width;
                  partial_rect.height = image_rect.height;

                  auto texture = atlas_textures[atlas_id];
                  if (placement.source_rect != placement.full_source_rect)
                  {
                    Vector2i fragment_offset(placement.source_rect.left - placement.full_source_rect.left,
                                             placement.source_rect.top - placement.full_source_rect.top);

                    map_interface.map_texture_fragment(resource_id, texture,
                                                       intersection(partial_rect, placement.atlas_rect),
                                                       fragment_offset);
                  }

                  else
                  {
                    map_interface.map_texture(resource_id, texture, partial_rect);
                  }
                }
              }

              ++atlas_id;
            }
          };

          // For all tiles in the tile library...
          // Map the tile id to a texture id.
          for (const auto& tile : tile_library.tiles())
          {
            map_resource_textures(texture_map.tile_id(tile.id), tile.image_file, tile.image_rect);
          }

          for (const auto& texture : texture_library.textures())
          {
            map_resource_textures(texture_map.texture_id(texture.id), texture.file_name, texture.rect);
          }
        }

        return texture_map;
      }

      PlacementMap generate_atlas_placement_map(const resources::Track& track, const ImageMapping& image_mapping,
                                                Vector2i atlas_size, bool include_all_assets)
      {
        const auto& tile_interface = track.tile_library().tiles();
        const auto& tile_group_interface = track.tile_library().tile_groups();

        const auto& texture_interface = track.texture_library().textures();

        AssetCache<resources::max_tile_id> tile_cache;
        AssetCache<resources::max_texture_id> texture_cache;

        utility::AtlasList atlas_list(atlas_size);

        // Make sure that fragmented entries have one pixel of overlap space, to make
        // sure we can draw them without artifacts.
        atlas_list.set_fragment_overlap(1);

        PlacementMap placement_map;
        placement_map.atlases.resize(atlas_list.atlas_count());
        auto max_atlas_rect_size = atlas_size / 2;

        auto store_entries = [&](const utility::AtlasEntry& entry, boost::string_ref file_name, 
                                 const IntRect& full_source_rect)
        {
          ensure_placement_map_size(placement_map, entry.atlas_id);

          auto& atlas = placement_map.atlases[entry.atlas_id];
          add_atlas_placement(atlas, file_name, entry.atlas_rect, entry.source_rect, full_source_rect);

          if (entry.source_rect != full_source_rect)
          {
            AtlasFragment fragment;
            fragment.atlas_id = entry.atlas_id;
            fragment.atlas_rect = entry.atlas_rect;
            fragment.source_rect = entry.source_rect;
            fragment.full_source_rect = full_source_rect;
            fragment.image_file = file_name;
            placement_map.atlas_fragments.push_back(fragment);
          }
        };

        auto store_entries_cached = [&](const utility::AtlasEntry& entry, boost::string_ref file_name, 
                                        IntRect full_source_rect, auto& asset_cache, std::size_t asset_id)
        {
          texture_cache.ensure_size(entry.atlas_id);
          tile_cache.ensure_size(entry.atlas_id);

          asset_cache.set(entry.atlas_id, asset_id);
          store_entries(entry, file_name, full_source_rect);
        };

        // Little helper lambda that only allocates space if the rect does not exist in the placement map yet.
        auto allocate_rect_if_required = [&](boost::string_ref file_name, const IntRect& source_rect,
                                             auto& asset_cache, std::size_t asset_id)
        {
          IntRect rect = find_enclosing_rect(image_mapping, file_name, source_rect);
          std::size_t atlas_id = atlas_list.current_atlas();

          if (!texture_rect_exists(placement_map.atlases[atlas_id], file_name, rect) &&
              !fragmented_texture_rect_exists(placement_map, file_name, rect))
          {
            auto callback = std::bind(store_entries_cached, std::placeholders::_1, file_name, rect,
                                      std::ref(asset_cache), asset_id);

            utility::allocate_atlas_rect(atlas_list, rect, max_atlas_rect_size, callback);
          }
        };

        // This algorithm goes as follows
        // * Iterate through the track layers and attempt to fit as many sub-images 
        // * into the current texture atlas as possible.
        // * This will use the image rects that we got from the image_mapping.
        // * If something does not fit, create a new texture atlas

        for (const auto& layer : track.layers())
        {
          for (const auto& geometry : layer.geometry)
          {
            std::size_t atlas_id = atlas_list.current_atlas();

            if (texture_cache.is_set(atlas_id, geometry.texture_id)) continue;

            auto texture_it = texture_interface.find(geometry.texture_id);
            if (texture_it != texture_interface.end())
            {
              allocate_rect_if_required(texture_it->file_name, texture_it->rect,
                                        texture_cache, texture_it->id);
            }
          }

          // Loop through all the placed tiles
          for (const auto& tile : layer.tiles)
          {
            std::size_t atlas_id = atlas_list.current_atlas();

            // Looking in the tile cache is a cheap operation, so we do that first.
            if (tile_cache.is_set(atlas_id, tile.id)) continue;

            auto tile_it = tile_interface.find(tile.id);

            // First see if there's a tile with this id
            if (tile_it != tile_interface.end())
            {
              allocate_rect_if_required(tile_it->image_file, tile_it->image_rect,
                                        tile_cache, tile.id);
            }

            // Otherwise, try to find a tile group with the given id
            else
            {
              auto tile_group_it = tile_group_interface.find(tile.id);
              if (tile_group_it != tile_group_interface.end())
              {
                // If found, loop through all sub-tiles nd just work the magic
                // through the tile_it variable. A bit ugly, but it'll have to do.
                for (auto sub_tile : tile_group_it->sub_tiles)
                {
                  // Another tight loop, do the cheap cache operation first.
                  if (tile_cache.is_set(atlas_list.current_atlas(), sub_tile.id)) continue;

                  auto sub_tile_it = tile_interface.find(sub_tile.id);
                  if (sub_tile_it != tile_interface.end())
                  {
                    allocate_rect_if_required(sub_tile_it->image_file, sub_tile_it->image_rect,
                                              tile_cache, sub_tile.id);
                  }
                }
              }
            }
          }
        }

        // If requested, allocate space for the tiles and textures that weren't used
        if (include_all_assets)
        {
          for (const auto& image_data : image_mapping)
          {
            auto image_file = image_data.first;

            for (auto rect : image_data.second)
            {
              if (!texture_rect_exists(placement_map, image_data.first, rect) &&
                  !fragmented_texture_rect_exists(placement_map, image_data.first, rect))
              {
                auto callback = std::bind(store_entries, std::placeholders::_1, image_file, rect);
                utility::allocate_atlas_rect(atlas_list, rect, max_atlas_rect_size, callback);
              }
            }
          }
        }

        // Set the atlas size - ideally we would do this when we create the atlases, but
        // I'm not sure how to pull that off with the code we have.
        for (std::size_t id = 0; id != atlas_list.atlas_count(); ++id)
        {
          placement_map.atlases[id].size = atlas_list.atlas_size(id);
        }

        return placement_map;
      }


      TrackScene generate_track_scene(const resources::Track& track, const PlacementMap& placement_map)
      {
        ImageLoader image_loader;
        TrackScene track_scene(track.size());

        using texture_type = TextureMapping::texture_type;

        std::vector<texture_type> atlas_textures;
        atlas_textures.reserve(placement_map.atlases.size());

        for (const auto& atlas : placement_map.atlases)
        {
          auto surface = build_atlas_surface(atlas, image_loader);

          auto texture = std::make_unique<graphics::Texture>(graphics::create_texture_from_image(surface));

          glCheck(glBindTexture(GL_TEXTURE_2D, texture->get()));
          glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
          glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

          atlas_textures.push_back(track_scene.register_texture(std::move(texture)));
        }

        glCheck(glBindTexture(GL_TEXTURE_2D, 0));

        auto texture_mapping = generate_resource_texture_map(track, placement_map, atlas_textures.data());
        scene::build_track_vertices(track, texture_mapping, track_scene);

        return track_scene;
      }
    }
  }
}