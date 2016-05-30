/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "dynamic_scene_generator.hpp"
#include "dynamic_scene.hpp"

#include "graphics/texture.hpp"
#include "graphics/image_loader.hpp"
#include "graphics/image.hpp"

#include "stage/stage.hpp"

#include "world/car.hpp"

#include "utility/string_utilities.hpp"
#include "utility/vector2.hpp"
#include "utility/texture_atlas.hpp"

#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <map>

namespace ts
{
  namespace scene
  {
    using ImageLoader = graphics::ImageLoader<sf::Image, graphics::DefaultImageLoader>;

    namespace detail
    {
      static IntRect compute_frame_bounds(const sf::Image& image, Vector2i frame_size)
      {
        // Get the bounds of the opaque area in the first frame in image.
        // That is, the area defined by the minimum and maximum values that
        // are not fully transparent.

        std::int32_t min_x = frame_size.x, min_y = frame_size.y, max_x = 0, max_y = 0;

        for (std::int32_t y = 0; y != frame_size.y; ++y)
        {
          for (std::int32_t x = 0; x != frame_size.x; ++x)
          {
            if (image.getPixel(x, y).a == 0) continue;

            if (x < min_x) min_x = x;
            if (y < min_y) min_y = y;

            if (x > max_x) max_x = x;
            if (y > max_y) max_y = y;
          }
        }

        auto width = std::max(max_x - min_x, 0);
        auto height = std::max(max_y - min_y, 0);

        return IntRect(min_x, min_y, width, height);
      }

      struct AtlasEntry : utility::AtlasEntry
      {
        explicit AtlasEntry(utility::AtlasEntry base, boost::string_ref image_path_, Vector2i frame_size_)
          : utility::AtlasEntry(base),
          image_path(image_path_),
          frame_size(frame_size_)
        {}

        boost::string_ref image_path;
        Vector2i frame_size;
      };

      static std::vector<AtlasEntry> generate_car_model_atlas(const stage::StageDescription& stage_desc,
                                                              utility::AtlasList& atlas_list, ImageLoader& image_loader)
      {
        // This function attempts to put as many car images as it can
        // into as few textures texture as it can by using a texture atlas.

        std::size_t current_atlas = atlas_list.current_atlas();
        auto atlas_size = atlas_list.atlas_size();
        std::vector<AtlasEntry> image_mapping;

        // For every car model...
        for (const auto& model : stage_desc.car_models)
        {
          const auto& image = image_loader.load_image(model.image_path);
          auto image_size = image.getSize();

          // See if there are any mapping entries that match the image part of the current model.
          auto it = std::find_if(image_mapping.begin(), image_mapping.end(),
                                 [&](const AtlasEntry& entry)
          {
            return entry.image_path == model.image_path;
          });

          // If there is one, do nothing. Else...
          if (it == image_mapping.end())
          {
            // Create a new entry in the atlas.
            Vector2i frame_size(model.image_rect.width / model.num_rotations, model.image_rect.height);

            auto column_count = std::min<std::uint32_t>(atlas_size.x / frame_size.x, model.num_rotations);
            auto row_count = (model.num_rotations + column_count - 1) / column_count;

            IntRect source_rect(0, 0, column_count * frame_size.x, row_count * frame_size.y);

            auto entry = atlas_list.allocate_rect(source_rect);
            if (!entry)
            {
              // Atlas entry could not be allocated, create a new atlas and try again.
              // This has to succeed, or we have to bail out with an exception.
              current_atlas = atlas_list.create_atlas();
              entry = atlas_list.allocate_rect(source_rect);

              if (!entry) throw std::runtime_error("could not load car texture: file too large");              
            }

            // Store the result in the image mapping.
            image_mapping.emplace_back(*entry, model.image_path, frame_size);

            image_mapping.back().source_rect.width = image_size.x;
            image_mapping.back().source_rect.height = image_size.y;
          }
        }

        return image_mapping;
      }
    }

    std::unique_ptr<DynamicScene> generate_dynamic_scene(const stage::Stage& stage_object)
    {      
      const auto& stage_desc = stage_object.stage_description();   

      ImageLoader image_loader;
      auto dynamic_scene = std::make_unique<DynamicScene>();

      auto atlas_size = std::min(graphics::max_texture_size(), 2048);
      utility::AtlasList atlas_list({ atlas_size, atlas_size });

      auto texture_atlas = detail::generate_car_model_atlas(stage_desc, atlas_list, image_loader);

      // Use a vector to map atlas entries to dynamic scene models.
      std::vector<std::size_t> model_ids;
      const std::size_t car_model_index = 0;

      // Create the atlas images that we just generated the blueprints for.
      std::vector<sf::Image> atlas_images;
      atlas_images.resize(atlas_list.atlas_count());
      for (auto& image : atlas_images)
      {
        image.create(atlas_size, atlas_size, sf::Color::Transparent);
      }

      // Now, loop through the entries, look up the corresponding images and copy the image data over.
      for (const auto& entry : texture_atlas)
      {
        const auto& source_image = image_loader.load_image(entry.image_path);
        std::int32_t image_width = source_image.getSize().x;

        auto& dest_image = atlas_images[entry.atlas_id];

        auto atlas_rect = entry.atlas_rect;        
        sf::IntRect source_rect(entry.source_rect.left, entry.source_rect.top,
                                entry.atlas_rect.width, entry.frame_size.y);

        // This is not a 1:1 copy, if the source image is too wide for the 
        // texture image, it will be divided into portions which will be placed below each other.
        while (source_rect.left < entry.source_rect.right())
        {
          if (source_rect.left + source_rect.width > image_width)
          {
            source_rect.width = image_width - source_rect.left;
          }

          dest_image.copy(source_image, atlas_rect.left, atlas_rect.top, source_rect);

          source_rect.left += atlas_rect.width;
          atlas_rect.top += source_rect.height;
        }
      }
      
      // Now, use the image to create all the required car textures and register them to the 
      // dynamic scene object.
      std::vector<std::size_t> texture_ids;
      for (const auto& image : atlas_images)
      {
        auto texture = std::make_unique<graphics::Texture>(graphics::create_texture_from_image(image));
        
        Vector2i image_size(image.getSize().x, image.getSize().y);
        auto texture_id = dynamic_scene->register_texture(std::move(texture), image_size);

        texture_ids.push_back(texture_id);
      }      

      // Now, register the car models with the dynamic scene object, 
      for (const auto& model : stage_desc.car_models)
      {
        // Find the matching entry in the texture atlas.
        auto entry = std::find_if(texture_atlas.begin(), texture_atlas.end(),
                                  [&](const auto& entry)
        {
          return entry.image_path == model.image_path;
        });

        // All models should have an entry in the texture atlas by now, if we don't it's a logic error.
        if (entry == texture_atlas.end()) throw std::logic_error("logic error while generating car textures");          

        auto& image = image_loader.load_image(model.image_path);

        DynamicScene::TextureInfo texture_info;
        texture_info.frame_bounds = detail::compute_frame_bounds(image, entry->frame_size);
        texture_info.frame_size = entry->frame_size;
        texture_info.num_rotations = model.num_rotations;
        texture_info.texture_rect = entry->atlas_rect;
        texture_info.scale = model.image_scale;

        auto texture_id = texture_ids[entry->atlas_id];
        auto model_id = dynamic_scene->add_model(world::EntityType::Car, texture_id, texture_info);

        model_ids.push_back(model_id);
      }

      sf::Image color_scheme;
      color_scheme.loadFromFile("data/color_scheme.png");
      IntRect scheme_rect(0, 0, 32, 32);
      dynamic_scene->register_color_schemes(graphics::create_texture_from_image(color_scheme), { 32, 32 }, &scheme_rect, 1);
      
      // Now, loop through the car instances and add them to the scene one by one.
      const auto& world_object = stage_object.world();
      for (const auto& car_instance : stage_desc.car_instances)
      {
        if (auto car = world_object.find_car(car_instance.instance_id))
        {
          const auto& car_def = stage_desc.car_models[car_instance.model_id];

          // We stored the model id away, retrieve it.
          auto model_id = model_ids[car_model_index + car_instance.model_id];

          resources::ColorScheme color_scheme;
          color_scheme.scheme_id = 0;
          color_scheme.colors[0] = { 255, 255, 255, 255 };          
          color_scheme.colors[1] = { 255, 150, 0, 255 };
          color_scheme.colors[2] = { 180, 0, 0, 255 };
          dynamic_scene->add_entity(car, model_id, color_scheme);
        }
      }

      return dynamic_scene;
    }
  }
}