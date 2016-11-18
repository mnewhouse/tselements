/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "dynamic_scene.hpp"
#include "drawable_entity.hpp"

#include "world/entity.hpp"

#include "resources/color_scheme.hpp"

#include "utility/debug_log.hpp"
#include "utility/rotation.hpp"

#include "graphics/gl_check.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <boost/optional.hpp>

#include <cmath>
#include <algorithm>
#include <array>
#include <iostream>

namespace ts
{
  namespace scene
  {
    namespace detail
    {
      static constexpr std::int32_t num_distinct_rotations = 1024;

      struct Texture
      {
        std::unique_ptr<graphics::Texture> texture;
        Vector2i texture_size;
      };

      struct EntityModel
      {
        world::EntityType type;
        const graphics::Texture* texture;
        Vector2i texture_size;        
        std::uint32_t num_rotations; 

        struct EntityInfoCache
        {
          std::array<DrawableEntity::Vertex, 4> vertices;

          glm::mat4 model_matrix;
          glm::mat4 colorizer_matrix;          
        };

        double cache_index_multiplier;
        std::vector<EntityInfoCache> entity_info_cache;
      };

      struct EntityInstance
      {
        std::uintptr_t type_tag;
        const world::Entity* entity;
        Vector2<double> stored_position;
        std::size_t model_id;

        struct ColorScheme
        {
          std::uint16_t scheme_id = 0;
          std::array<Colorf, 3> colors;
        };


        boost::optional<ColorScheme> color_scheme;
      };

      // This function can be used to calculate the frame id given a rotation and number of frames,
      // and also gives back the amount that the rotation deviates from the frame's corresponding rotation.
      static auto calculate_frame_id(Rotation<double> rotation, std::uint32_t frame_count, 
                                     Rotation<double>& deviation)
      {
        // Degrees in range [-180, 180]
        rotation.normalize();

        auto deg = rotation.degrees();
        auto dbl_frame_count = static_cast<double>(frame_count);

        auto rotation_range = 360.0 / dbl_frame_count;
        auto frame_id = std::round(deg / rotation_range);
        auto frame_rotation = degrees(frame_id * rotation_range);

        deviation = rotation - frame_rotation;

        if (frame_id < 0.0) frame_id += dbl_frame_count;
        return static_cast<std::int32_t>(frame_id);
      }
    }

    struct DynamicScene::Impl
    {
      std::vector<detail::Texture> textures;
      std::vector<detail::EntityModel> entity_models;
      std::vector<detail::EntityInstance> entities;

      graphics::Texture color_scheme_texture;
      std::vector<FloatRect> color_scheme_rects;
    };

    DynamicScene::DynamicScene()
      : impl_(std::make_unique<Impl>())
    {
    }

    DynamicScene::~DynamicScene() = default;

    DynamicScene::DynamicScene(DynamicScene&& other) = default;
    DynamicScene& DynamicScene::operator=(DynamicScene&& rhs) = default;

    std::size_t DynamicScene::register_texture(std::unique_ptr<graphics::Texture> texture, Vector2i size)
    {
      glCheck(glBindTexture(GL_TEXTURE_2D, texture->get()));
      glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      glCheck(glBindTexture(GL_TEXTURE_2D, 0));

      std::size_t texture_id = impl_->textures.size();
      impl_->textures.push_back({ std::move(texture), size });
      
      return texture_id;
    }

    std::size_t DynamicScene::add_model(world::EntityType entity_type, std::size_t texture_id, const TextureInfo& texture_info)
    {
      std::size_t model_id = impl_->entity_models.size();

      detail::EntityModel model;
      model.type = entity_type;
      model.texture = impl_->textures[texture_id].texture.get();
      model.texture_size = impl_->textures[texture_id].texture_size;
      model.num_rotations = texture_info.num_rotations;

      model.entity_info_cache.resize(detail::num_distinct_rotations + 1);
      model.cache_index_multiplier = detail::num_distinct_rotations / Rotation<double>::double_pi;

      auto frame_size = vector2_cast<float>(texture_info.frame_size);
      auto row_width = texture_info.texture_rect.width / texture_info.frame_size.x;

      auto inverse_index_multiplier = 1.0 / model.cache_index_multiplier;
      auto inverse_texture_size = 1.0f / vector2_cast<float>(model.texture_size);
      auto scale = frame_size / static_cast<float>(texture_info.scale) * 0.5f;

      auto frame_bounds = rect_cast<float>(texture_info.frame_bounds);
      frame_bounds.left /= frame_size.x;
      frame_bounds.top /= frame_size.y;
      frame_bounds.width /= frame_size.x;
      frame_bounds.height /= frame_size.y;

      auto colorizer_top_left = make_vector2(-frame_bounds.left * frame_bounds.width,
                                             -frame_bounds.top * frame_bounds.height);

      auto colorizer_bottom_right = make_vector2(1.0f + (1.0f - frame_bounds.right()) * frame_bounds.width,
                                                 1.0f + (1.0f - frame_bounds.bottom()) * frame_bounds.height);

      const glm::vec3 scale_3d(scale.x, scale.y, 1.0f);
      const glm::vec3 rotation_axis(0.0f, 0.0f, 1.0f);

      std::int32_t index = 0;
      for (auto& cache_entry : model.entity_info_cache)
      {
        auto rotation = radians(index * inverse_index_multiplier), deviation = radians(0.0);
        rotation.normalize();

        auto frame_id = detail::calculate_frame_id(rotation, model.num_rotations, deviation);

        auto rotation_radians = static_cast<float>(rotation.radians());
        auto deviation_radians = static_cast<float>(deviation.radians());

        cache_entry.model_matrix = glm::scale(glm::rotate(deviation_radians, rotation_axis), scale_3d);
        cache_entry.colorizer_matrix = glm::rotate(rotation_radians, rotation_axis);

        cache_entry.vertices[0].position = { -1.0f, -1.0f };
        cache_entry.vertices[1].position = { -1.0f, 1.0f };
        cache_entry.vertices[2].position = { 1.0f, -1.0f };
        cache_entry.vertices[3].position = { 1.0f, 1.0f };

        {
          auto texture_position = make_vector2(frame_id % row_width, frame_id / row_width) * texture_info.frame_size;

          auto top_left = vector2_cast<float>(texture_position) * inverse_texture_size;
          auto bottom_right = vector2_cast<float>(texture_position + texture_info.frame_size) * inverse_texture_size;

          cache_entry.vertices[0].texture_coords = { top_left.x, top_left.y };
          cache_entry.vertices[1].texture_coords = { top_left.x, bottom_right.y };
          cache_entry.vertices[2].texture_coords = { bottom_right.x, top_left.y };
          cache_entry.vertices[3].texture_coords = { bottom_right.x, bottom_right.y };
        }

        {
          auto& transform = cache_entry.colorizer_matrix;

          auto top_left = transform * glm::vec4(colorizer_top_left.x, colorizer_top_left.y, 0.0f, 1.0f);
          auto bottom_left = transform * glm::vec4(colorizer_top_left.x, colorizer_bottom_right.y, 0.0f, 1.0f);
          auto top_right = transform * glm::vec4(colorizer_bottom_right.x, colorizer_top_left.y, 0.0f, 1.0f);
          auto bottom_right = transform * glm::vec4(colorizer_bottom_right.x, colorizer_bottom_right.y, 0.0f, 1.0f);

          cache_entry.vertices[0].colorizer_coords = { top_left.x, top_left.y, 0.0f };
          cache_entry.vertices[1].colorizer_coords = { bottom_left.x, bottom_left.y, 0.0f };
          cache_entry.vertices[2].colorizer_coords = { top_right.x, top_right.y, 0.0f };
          cache_entry.vertices[3].colorizer_coords = { bottom_right.x, bottom_right.y, 0.0f };
        }
        
        ++index;
      }

      // And add the "base" rotation once more to the end
      auto front = model.entity_info_cache.front();
      model.entity_info_cache.push_back(front);
      
      impl_->entity_models.push_back(std::move(model));
      return model_id;
    }

    void DynamicScene::register_color_schemes(graphics::Texture texture, Vector2i texture_size,
                                              const IntRect* scheme_rects, std::size_t scheme_count)
    {
      auto width_multiplier = 1.0f / texture_size.x;
      auto height_multiplier = 1.0f / texture_size.y;

      impl_->color_scheme_rects.resize(scheme_count);
      std::transform(scheme_rects, scheme_rects + scheme_count, impl_->color_scheme_rects.begin(),
                     [=](const IntRect& rect) -> FloatRect
      {
        return
        {
          rect.left * width_multiplier,
          rect.top * height_multiplier,
          rect.width * width_multiplier,
          rect.height * height_multiplier
        };
      });

      impl_->color_scheme_texture = std::move(texture);
    }

    const graphics::Texture* DynamicScene::color_scheme_texture() const
    {
      return &impl_->color_scheme_texture;
    }

    void DynamicScene::add_entity(const world::Entity* entity, std::size_t model_id)
    {
      detail::EntityInstance instance;
      instance.entity = entity;
      instance.model_id = model_id;
      instance.stored_position = entity->position();

      impl_->entities.push_back(instance);
    }

    void DynamicScene::add_entity(const world::Entity* entity, std::size_t model_id, const resources::ColorScheme& color_scheme)
    {
      detail::EntityInstance instance;
      instance.entity = entity;
      instance.model_id = model_id;
      instance.stored_position = entity->position();
      instance.color_scheme.emplace();
      instance.color_scheme->scheme_id = color_scheme.scheme_id;
      std::transform(color_scheme.colors.begin(), color_scheme.colors.end(), instance.color_scheme->colors.begin(),
                     [](auto color) { return to_colorf(color); });
      
      impl_->entities.push_back(instance);
    }

    EntityModel DynamicScene::model_info(std::size_t model_id) const
    {
      const auto& model = impl_->entity_models[model_id];

      EntityModel model_info;
      model_info.texture = model.texture;
      model_info.type = model.type;
      return model_info;
    }

    std::size_t DynamicScene::model_count() const
    {
      return impl_->entity_models.size();
    }

    void DynamicScene::update_entity_positions()
    {
      for (auto& instance : impl_->entities)
      {
        instance.stored_position = instance.entity->position();
      }      
    }

    DrawableEntity DynamicScene::entity_info(std::size_t instance_id) const
    {
      const auto& instance = impl_->entities[instance_id];
      const auto& model = impl_->entity_models[instance.model_id];

      auto entity = instance.entity;
      auto rotation = entity->rotation();

      rotation.normalize();
      auto cache_index = static_cast<std::int32_t>(rotation.radians() * model.cache_index_multiplier);
      if (cache_index < 0) cache_index += detail::num_distinct_rotations;

      DrawableEntity drawable_entity;
      drawable_entity.entity = entity;
      drawable_entity.texture = model.texture;

      auto& cache_entry = model.entity_info_cache[cache_index];

      auto position = vector2_cast<float>(instance.stored_position);
      auto new_position = vector2_cast<float>(entity->position());

      auto translation = glm::translate(glm::vec3(position.x, position.y, 0.0f));
      auto new_translation = glm::translate(glm::vec3(new_position.x, new_position.y, 0.0f));

      drawable_entity.vertices = cache_entry.vertices;
      drawable_entity.colorizer_matrix = cache_entry.colorizer_matrix;
      drawable_entity.model_matrix = translation * cache_entry.model_matrix;
      drawable_entity.new_model_matrix = new_translation * cache_entry.model_matrix;

      // Loop through vertices to assign colorizer texture z-coord.

      drawable_entity.z_level = entity->z_level();
      drawable_entity.shadow_offset = static_cast<float>(entity->hover_distance());
      drawable_entity.new_shadow_offset = drawable_entity.shadow_offset;

      if (instance.color_scheme)
      {
        drawable_entity.colors = instance.color_scheme->colors;
      }

      else
      {
        const Colorf white(1.0f, 1.0f, 1.0f, 1.0f);
        drawable_entity.colors = { white, white, white };
      }

      return drawable_entity;
    }

    std::size_t DynamicScene::entity_count() const
    {
      return impl_->entities.size();
    }
  }
}