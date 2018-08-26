/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "resources/geometry.hpp"

#include "graphics/texture.hpp"
#include "graphics/shader.hpp"
#include "graphics/buffer.hpp"

#include <SFML/Graphics/Transform.hpp>

namespace ts
{
  namespace resources
  {
    struct PlacedTile;
  }

  namespace scene
  {
    class TextureMapping;
  }

  namespace editor
  {
    class TileInteractionRenderer
    {
    public:
      TileInteractionRenderer();

      void set_transform(const sf::Transform& model_matrix);
      void render(const sf::Transform& view_matrix, Vector2f track_size) const;

      void update_tile_geometry(const resources::PlacedTile*, std::size_t tile_count,
                                const scene::TextureMapping& texture_mapping);

      void clear_tile_geometry();

    private:
      void assign_vao_state();

      graphics::ShaderProgram selected_geometry_shader_;
      graphics::ShaderProgram default_geometry_shader_;

      graphics::Buffer vertex_buffer_;
      graphics::Buffer index_buffer_;
      graphics::VertexArray vertex_array_;

      std::uint32_t default_view_matrix_location_;
      std::uint32_t default_sampler_location_;
      std::uint32_t default_min_corner_location_;
      std::uint32_t default_max_corner_location_;
      std::uint32_t selected_view_matrix_location_;
      std::uint32_t selected_sampler_location_;

      struct Component
      {
        const graphics::Texture* texture = nullptr;
        const std::uint32_t* buffer_offset = nullptr;
        std::uint32_t element_count = 0;        
      };

      std::vector<Component> render_components_;      
      std::vector<resources::Vertex> vertex_cache_;
      std::vector<resources::Face> face_cache_;

      sf::Transform model_matrix_;
    };
  }
}
