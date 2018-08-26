/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "tile_interaction_renderer.hpp"

#include "resources/tiles.hpp"

#include "scene/scene_shaders.hpp"
#include "scene/texture_mapping.hpp"
#include "scene/track_vertices.hpp"

namespace ts
{
  namespace editor
  {
    static graphics::ShaderProgram create_selected_geometry_shader()
    {
      return{};
    }

    static graphics::ShaderProgram create_default_geometry_shader()
    {
    }

    TileInteractionRenderer::TileInteractionRenderer()
      : vertex_buffer_(graphics::create_buffer()),
        index_buffer_(graphics::create_buffer()),

        vertex_array_(graphics::create_vertex_array())
    {
      auto prog = graphics::create_shader_program(scene::shaders::track_vertex_shader,
                                                  scene::shaders::basic_track_fragment_shader);


      glBindAttribLocation(prog.get(), 0, "in_position");
      glBindAttribLocation(prog.get(), 1, "in_texCoords");

      graphics::link_shader_program(prog);
      default_geometry_shader_ = std::move(prog);

      default_view_matrix_location_ = glGetUniformLocation(default_geometry_shader_.get(), "u_viewMatrix");
      default_sampler_location_ = glGetUniformLocation(default_geometry_shader_.get(), "u_textureSampler");

      assign_vao_state();
    }

    void TileInteractionRenderer::assign_vao_state()
    {
      glCheck(glBindVertexArray(vertex_array_.get()));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_.get()));

      glCheck(glEnableVertexAttribArray(0));
      glCheck(glEnableVertexAttribArray(1));

      using V = resources::Vertex;
      glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V),
                                    reinterpret_cast<const void*>(static_cast<std::uintptr_t>(offsetof(V, position)))));

      glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(V),
                                    reinterpret_cast<const void*>(static_cast<std::uintptr_t>(offsetof(V, texture_coords)))));

      glCheck(glBindVertexArray(0));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    void TileInteractionRenderer::render(const sf::Transform& view_matrix, Vector2f track_size) const
    {
      glCheck(glUseProgram(default_geometry_shader_.get()));

      auto matrix = view_matrix * model_matrix_;
      glCheck(glUniformMatrix4fv(default_view_matrix_location_, 1, GL_FALSE, matrix.getMatrix()));
      glCheck(glUniform1i(default_sampler_location_, 0));

      glCheck(glBindVertexArray(vertex_array_.get()));  

      for (auto component : render_components_)
      {
        glCheck(glActiveTexture(GL_TEXTURE0));
        glCheck(glBindTexture(GL_TEXTURE_2D, component.texture->get()));
        glCheck(glDrawElements(GL_TRIANGLES, component.element_count, GL_UNSIGNED_INT, component.buffer_offset));
      }

      glCheck(glBindVertexArray(0));
      glCheck(glUseProgram(0));
    }

    void TileInteractionRenderer::update_tile_geometry(const resources::PlacedTile* placed_tiles, std::size_t tile_count,
                                                       const scene::TextureMapping& texture_mapping)
    {
      face_cache_.clear();
      vertex_cache_.clear();
      render_components_.clear();

      auto buffer_offset = reinterpret_cast<const std::uint32_t*>(std::uintptr_t(0));
      for (std::size_t i = 0; i != tile_count; ++i)
      {
        const auto& tile = placed_tiles[i];
        auto mapping_range = texture_mapping.find(texture_mapping.tile_id(tile.id));
        for (const auto& mapping : mapping_range)
        {
          auto vertices = scene::generate_tile_vertices(tile, *tile.definition, mapping.texture_rect, mapping.fragment_offset,
                                                        1.0f / mapping.texture->size());

          auto faces = scene::generate_tile_faces(static_cast<std::uint32_t>(vertex_cache_.size()));
          
          vertex_cache_.insert(vertex_cache_.end(), vertices.begin(), vertices.end());
          face_cache_.insert(face_cache_.end(), faces.begin(), faces.end());

          auto element_count = static_cast<std::uint32_t>(faces.size() * 3);

          if (render_components_.empty() || render_components_.back().texture != mapping.texture)
          {
            Component component;
            component.buffer_offset = buffer_offset;
            component.element_count = element_count;
            component.texture = mapping.texture;
            render_components_.push_back(component);
          }

          else
          {
            render_components_.back().element_count += element_count;
          }

          buffer_offset += element_count;
        }        
      }

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_.get()));

      glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_cache_.size() * sizeof(vertex_cache_.front()),
                           vertex_cache_.data(), GL_STATIC_DRAW));

      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, face_cache_.size() * sizeof(face_cache_.front()),
                           face_cache_.data(), GL_STATIC_DRAW));

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    void TileInteractionRenderer::clear_tile_geometry()
    {
      render_components_.clear();
    }

    void TileInteractionRenderer::set_transform(const sf::Transform& matrix)
    {
      model_matrix_ = matrix;
    }
  }
}