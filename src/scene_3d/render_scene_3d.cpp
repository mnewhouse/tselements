/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "render_scene_3d.hpp"
#include "viewport_3d.hpp"

#include "resources_3d/track_3d.hpp"
#include "resources_3d/elevation_map_3d.hpp"
#include "resources_3d/terrain_model_3d.hpp"
#include "resources_3d/model_3d.hpp"
#include "resources_3d/path_vertices_3d.hpp"

#include "graphics/gl_check.hpp"
#include "graphics/gl_scissor_box.hpp"
#include "graphics/image_loader.hpp"

#include "utility/math_utilities.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <numeric>

namespace ts
{
  namespace scene3d
  {
    RenderScene::RenderScene(const resources3d::Track& track)
      : terrain_shader_program_(graphics::create_shader_program(shaders::terrain_vertex_shader, shaders::terrain_fragment_shader)),       
        terrain_vertex_buffer_(graphics::create_buffer()),
        terrain_element_buffer_(graphics::create_buffer()),
        terrain_vertex_array_(graphics::create_vertex_array()),
        terrain_uniforms_(shaders::terrain_shader_uniform_locations(terrain_shader_program_))
    {
    }

    void RenderScene::load_textures(const resources3d::TextureLibrary& tex_library)
    {
    }
    
    void RenderScene::load_terrain_geometry(const resources3d::Track& track)
    {
      const auto& elevation_map = track.elevation_map();
      const auto world_size = track.size();
      const auto additional_terrain_size = Vector2f(5000.0f, 5000.0f);

      terrain_texture_ = &generic_textures_.front();
      auto texture_scale = 4.0f / make_2d(terrain_texture_->size());

      /*
      auto vertex_data_size = terrain_model.vertices.size() * sizeof(Vertex);
      auto element_data_size = terrain_model.faces.size() * sizeof(Face);

      auto vertex_buffer_size = utility::next_power_of_two(vertex_data_size);
      auto element_buffer_size = utility::next_power_of_two(element_data_size);

      auto vao = terrain_vertex_array_.get();
      glBindVertexArray(terrain_vertex_array_.get());

      glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_buffer_.get());
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_element_buffer_.get());
      
      glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, nullptr, GL_STATIC_DRAW));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, nullptr, GL_STATIC_DRAW));

      glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_data_size, terrain_model.vertices.data()));
      glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, element_data_size, terrain_model.faces.data()));

      glCheck(glEnableVertexAttribArray(0));
      glCheck(glEnableVertexAttribArray(1));
      glCheck(glEnableVertexAttribArray(2));
      glCheck(glEnableVertexAttribArray(3));

      glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, position))));

      glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, tex_coords))));

      glCheck(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, normal))));

      glCheck(glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, color))));

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 1);
      glBindVertexArray(0);
      */
    }

    void RenderScene::render(const Viewport& view_port, Vector2i screen_size, double frame_progress) const
    {
      glCheck(glEnable(GL_TEXTURE_2D));
      glCheck(glDisable(GL_CULL_FACE));
      glCheck(glDisable(GL_DEPTH_TEST));
      glCheck(glEnable(GL_BLEND));
      glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

      auto screen_rect = view_port.screen_rect();

      glCheck(glViewport(screen_rect.left, screen_size.y - screen_rect.bottom(),
                         screen_rect.width, screen_rect.height));

      graphics::scissor_box(view_port.screen_rect(), screen_size);      

      auto mat = projection_matrix(view_port);

      glUseProgram(terrain_shader_program_.get());

      glUniformMatrix4fv(terrain_uniforms_.view_matrix, 1, GL_FALSE, glm::value_ptr(mat));
      glUniform3f(terrain_uniforms_.light_direction, 0.0f, 0.0f, 1.0f);
      glUniform1i(terrain_uniforms_.texture_sampler, 0);

      glBindVertexArray(terrain_vertex_array_.get());
      glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_buffer_.get());
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_element_buffer_.get());

      using Vertex = resources3d::Vertex;
      
      glActiveTexture(GL_TEXTURE0);
      for (const auto& component : terrain_components_)
      {
        glBindTexture(GL_TEXTURE_2D_ARRAY, component.texture->get());

        glCheck(glDrawElements(GL_TRIANGLES, component.element_count, GL_UNSIGNED_INT, component.element_index));
      }     

      graphics::disable_scissor_box();

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      glUseProgram(0);
    }

    void RenderScene::update(std::int32_t frame_duration)
    {

    }
  }
}