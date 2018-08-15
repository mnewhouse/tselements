/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "dynamic_scene.hpp"
#include "drawable_entity.hpp"

#include "world/entity.hpp"

#include "resources/color_scheme.hpp"

#include "utility/debug_log.hpp"
#include "utility/rotation.hpp"

#include "graphics/gl_check.hpp"

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

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
      static constexpr std::int32_t max_color_schemes = 256;

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
        
        Vector2f scale;
        std::vector<std::pair<Vector2f, Vector2f>> texture_coords;
        sf::Transform colorizer_transform;
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

      std::vector<graphics::Texture> color_scheme_textures;
    };

    DynamicScene::DynamicScene()
      : impl_(std::make_unique<Impl>())
    {
      impl_->color_scheme_textures.reserve(detail::max_color_schemes);
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

      auto texture_size = vector2_cast<float>(impl_->textures[texture_id].texture_size);
      auto inverse_texture_size = 1.0f / texture_size;
      auto frame_size = vector2_cast<float>(texture_info.frame_size);
      auto row_width = texture_info.texture_rect.width / texture_info.frame_size.x;

      detail::EntityModel model;
      model.type = entity_type;
      model.texture = impl_->textures[texture_id].texture.get();
      model.texture_size = impl_->textures[texture_id].texture_size;
      model.num_rotations = texture_info.num_rotations;
      model.scale = frame_size / static_cast<float>(texture_info.scale);

      if (texture_info.frame_bounds.width > 0 && texture_info.frame_bounds.height > 0)
      {
        auto frame_bounds = rect_cast<float>(texture_info.frame_bounds);
        auto inv_frame_size = 1.0 / frame_size;

        frame_bounds.left *= inv_frame_size.x;
        frame_bounds.top *= inv_frame_size.y;
        frame_bounds.width *= inv_frame_size.x;
        frame_bounds.height *= inv_frame_size.y;

        frame_bounds.left -= 0.5f;
        frame_bounds.top -= 0.5f;

        auto center = make_vector2(frame_bounds.left + frame_bounds.width * 0.5f,
                                   frame_bounds.top + frame_bounds.height * 0.5f);

        model.colorizer_transform.
          translate(-center.x + 0.5, -center.y + 0.5).
          scale(1.0 / frame_bounds.width, 1.0 / frame_bounds.height);
      }

      model.texture_coords.resize(texture_info.num_rotations);
      for (std::uint32_t frame_id = 0; frame_id < texture_info.num_rotations; ++frame_id)
      {
        auto texture_position = make_vector2(frame_id % row_width, frame_id / row_width) * texture_info.frame_size;

        auto& coords = model.texture_coords[frame_id];
        coords.second = vector2_cast<float>(texture_info.frame_size) * inverse_texture_size;
        coords.first = vector2_cast<float>(texture_position) * inverse_texture_size + coords.second * 0.5f;        
      }
      
      impl_->entity_models.push_back(std::move(model));
      return model_id;
    }

    std::uint32_t DynamicScene::register_color_scheme(graphics::Texture texture)
    {
      auto& textures = impl_->color_scheme_textures;
      if (textures.size() == textures.capacity()) return static_cast<std::uint32_t>(-1);

      glBindTexture(GL_TEXTURE_2D, texture.get());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      auto id = static_cast<std::uint32_t>(textures.size());
      textures.push_back(std::move(texture));
      return id;
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
      auto new_position = entity->position();

      DrawableEntity drawable_entity;
      drawable_entity.entity = entity;
      drawable_entity.texture = model.texture;

      drawable_entity.colors.fill(1.0f);
      drawable_entity.colorizer_texture = nullptr;

      Rotation<double> deviation;
      auto frame_id = detail::calculate_frame_id(rotation, model.num_rotations, deviation);

      drawable_entity.texture_coords_offset = model.texture_coords[frame_id].first;
      drawable_entity.texture_coords_scale = model.texture_coords[frame_id].second;

      drawable_entity.model_transform = sf::Transform().
        translate(instance.stored_position.x, instance.stored_position.y).
        rotate(deviation.degrees()).
        scale(model.scale.x, model.scale.y);

      drawable_entity.new_model_transform = sf::Transform().
        translate(new_position.x, new_position.y).
        rotate(deviation.degrees()).
        scale(model.scale.x, model.scale.y);

      drawable_entity.colorizer_transform.
        combine(model.colorizer_transform).
        rotate(-rotation.degrees() + deviation.degrees());
        

      drawable_entity.level = entity->z_level();
      drawable_entity.shadow_offset = static_cast<float>(entity->hover_distance());
      drawable_entity.new_shadow_offset = drawable_entity.shadow_offset;

      if (instance.color_scheme)
      {
        auto scheme_id = instance.color_scheme->scheme_id;
        if (scheme_id < impl_->color_scheme_textures.size())
        {
          drawable_entity.colorizer_texture = &impl_->color_scheme_textures[scheme_id];
          
          std::uint32_t color_idx = 0;
          for (auto c : instance.color_scheme->colors)
          {
            drawable_entity.colors[color_idx++] = c.r;
            drawable_entity.colors[color_idx++] = c.g;
            drawable_entity.colors[color_idx++] = c.b;
          }
        }
      }

      return drawable_entity;
    }

    std::size_t DynamicScene::entity_count() const
    {
      return impl_->entities.size();
    }
  }
}