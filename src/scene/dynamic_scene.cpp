/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "dynamic_scene.hpp"
#include "drawable_entity.hpp"

#include "world/entity.hpp"

#include "resources/color_scheme.hpp"

#include "utility/debug_log.hpp"
#include "utility/rotation.hpp"

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
      struct Texture
      {
        std::unique_ptr<graphics::Texture> texture;
        Vector2i texture_size;
      };

      struct EntityModel
      {
        world::EntityType type;
        const graphics::Texture* texture;
        Vector2<float> texture_size;
        Vector2<float> inverse_texture_size;
        Vector2<float> frame_size;
        Rect<float> frame_bounds;
        
        Vector2i texture_position;
        std::uint32_t num_rotations;
        std::uint32_t row_width;        

        float inverse_scale;
      };

      struct EntityInstance
      {
        std::uintptr_t type_tag;
        const world::Entity* entity;
        Vector2<double> stored_position;
        std::size_t model_id;

        boost::optional<resources::ColorScheme> color_scheme;
      };

      // This function can be used to calculate the frame id given a rotation and number of frames,
      // and also gives back the amount that the rotation deviates from the frame's corresponding rotation.
      static std::uint32_t calculate_frame_id(Rotation<double> rotation, std::uint32_t frame_count, 
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
        return static_cast<std::uint32_t>(frame_id);
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

    DynamicScene::~DynamicScene()
    {
    }

    std::size_t DynamicScene::register_texture(std::unique_ptr<graphics::Texture> texture, Vector2i size)
    {
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
      model.texture_size = vector2_cast<float>(impl_->textures[texture_id].texture_size);
      model.inverse_texture_size = 1.0f / model.texture_size;
      model.texture_position = { texture_info.texture_rect.left, texture_info.texture_rect.width };
      model.inverse_scale = 1.0f / static_cast<float>(texture_info.scale);
      model.num_rotations = texture_info.num_rotations;
      model.frame_size = vector2_cast<float>(texture_info.frame_size);
      model.frame_bounds = 
      {
        texture_info.frame_bounds.left / model.frame_size.x,
        texture_info.frame_bounds.top / model.frame_size.y,
        texture_info.frame_bounds.width / model.frame_size.x,
        texture_info.frame_bounds.height / model.frame_size.y
      };
      model.row_width = texture_info.texture_rect.width / texture_info.frame_size.x;

      impl_->entity_models.push_back(model);
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
      instance.color_scheme = color_scheme;
      
      impl_->entities.push_back(instance);
    }

    EntityModel DynamicScene::model_info(std::size_t model_id) const
    {
      EntityModel model;
      model.texture = impl_->entity_models[model_id].texture;
      model.type = impl_->entity_models[model_id].type;
      return model;
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

      auto stored_position = vector2_cast<float>(instance.stored_position);

      auto entity = instance.entity;
      auto z_position = static_cast<float>(entity->z_position());
      auto rotation = entity->rotation();

      Rotation<double> deviation;
      auto frame_id = detail::calculate_frame_id(rotation, model.num_rotations, deviation);

      Vector2<float> texture_position((frame_id % model.row_width) * model.frame_size.x,
                                      (frame_id / model.row_width) * model.frame_size.y);

      DrawableEntity drawable_entity;
      drawable_entity.entity = entity;
      drawable_entity.texture = model.texture;
      drawable_entity.frame_offset = vector2_cast<float>(entity->position()) - stored_position;
      drawable_entity.frame_bounds = model.frame_bounds;

      drawable_entity.texture_coords =
      {
        texture_position * model.inverse_texture_size,
        model.frame_size * model.inverse_texture_size
      };

      if (instance.color_scheme)
      {
        auto radians = static_cast<float>(rotation.radians() - deviation.radians());
        auto sin = glm::sin(radians);
        auto cos = glm::cos(radians);

        drawable_entity.colorizer_coords = impl_->color_scheme_rects[instance.color_scheme->scheme_id];
        drawable_entity.colorizer_transformation =
        {
          cos, -sin,
          sin, cos
        };

        std::copy_n(instance.color_scheme->colors, 3, drawable_entity.colors);
      }

      auto scale = model.inverse_scale * model.texture_size;
      auto& transform = drawable_entity.transformation;

      transform = glm::translate(glm::vec3(stored_position.x, stored_position.y, 1.0f));
      transform = glm::rotate(transform, static_cast<float>(deviation.radians()), glm::vec3(0.0f, 0.0f, 1.0f));
      transform = glm::scale(transform, glm::vec3(scale.x, scale.y, 1.0f));

      return drawable_entity;
    }

    std::size_t DynamicScene::entity_count() const
    {
      return impl_->entities.size();
    }
  }
}