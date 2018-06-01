/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "drawable_entity.hpp"

#include "graphics/texture.hpp"

#include "world/entity_type.hpp"

#include "utility/rect.hpp"
#include "utility/vertex.hpp"

#include <vector>
#include <memory>
#include <cstdint>
#include <typeindex>

namespace ts
{
  namespace world
  {
    class Entity;
  }

  namespace resources
  {
    struct ColorScheme;
  }

  namespace scene
  {
    struct EntityModel
    {
      world::EntityType type;
      const graphics::Texture* texture;
    };

    // The DynamicScene is used to keep information about the graphical state of all dynamic objects,
    // such as cars, projectiles, and dynamic scenery objects.
    class DynamicScene
    {
    public:
      DynamicScene();
      ~DynamicScene();

      DynamicScene(DynamicScene&&);
      DynamicScene& operator=(DynamicScene&&);

      // Register a texture
      std::size_t register_texture(std::unique_ptr<graphics::Texture> texture, Vector2i texture_size);

      struct TextureInfo
      {
        IntRect texture_rect;
        Vector2i frame_size;
        IntRect frame_bounds;
        std::uint32_t num_rotations;
        double scale;
      };

      // Adds a model to the list and returns the id.
      // The texture rect must fully contain all frames, `num_rotations` in total.
      // The frames are assumed to be laid out left-to-right, as many as can fit
      // on one row, followed by the next rows.
      // This means that the caller must make sure the following expression is true:
      // (texture_rect.width / frame_size.x) * (texture_rect.height / frame_size.x) >= num_rotations.
      std::size_t add_model(world::EntityType entity_type, std::size_t texture_id, const TextureInfo& texture_info);

      EntityModel model_info(std::size_t model_id) const;
      std::size_t model_count() const;

      DrawableEntity entity_info(std::size_t instance_id) const;
      std::size_t entity_count() const;

      void register_color_schemes(graphics::Texture texture, Vector2i texture_size,
                                  const IntRect* schemerects, std::size_t num_schemes);

      const graphics::Texture* color_scheme_texture() const;

      void update_entity_positions();

      // Adds an entity to the list of drawables.
      void add_entity(const world::Entity* entity, std::size_t model);

      void add_entity(const world::Entity* entity, std::size_t model, const resources::ColorScheme& color_scheme);

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}
