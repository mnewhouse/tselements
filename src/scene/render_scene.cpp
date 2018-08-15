/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/


#include "render_scene.hpp"
#include "scene_shaders.hpp"
#include "dynamic_scene.hpp"
#include "view_matrix.hpp"

#include "graphics/gl_scissor_box.hpp"
#include "graphics/gl_check.hpp"

#include "world/world_limits.hpp"

#include "utility/math_utilities.hpp"

#include <GL/glew.h>
#include <GL/GL.h>

namespace ts
{
  namespace scene
  {
    using namespace render_scene;

    struct CarVertex
    {
      Vector2f position;
      Vector2f texture_coords;
    };
    
    RenderScene::RenderScene(TrackScene track_scene)
      : track_scene_(std::move(track_scene))
    {
      load_shader_programs();

      reload_track_components();

      setup_entity_buffers();

      glCheck(glFinish());
    }

    void RenderScene::load_shader_programs()
    {
      {
        track_shader_program_ = graphics::create_shader_program(shaders::track_vertex_shader,
                                                                shaders::track_fragment_shader);

        auto prog = track_shader_program_.get();

        auto& locations = track_component_locations_;
        locations.in_position = glCheck(glGetAttribLocation(prog, "in_position"));
        locations.in_texCoords = glCheck(glGetAttribLocation(prog, "in_texCoords"));
        
        locations.view_matrix = glCheck(glGetUniformLocation(prog, "u_viewMatrix"));
        locations.texture_sampler = glCheck(glGetUniformLocation(prog, "u_textureSampler"));
        locations.min_corner = glCheck(glGetUniformLocation(prog, "u_minCorner"));
        locations.max_corner = glCheck(glGetUniformLocation(prog, "u_maxCorner"));
      }

      {
        car_shader_program_ = graphics::create_shader_program(shaders::car_vertex_shader,
                                                              shaders::car_fragment_shader);

        auto prog = car_shader_program_.get();
        auto& locations = car_locations_;
        locations.in_position = glCheck(glGetAttribLocation(prog, "in_position"));
        locations.in_texCoords = glCheck(glGetAttribLocation(prog, "in_texCoords"));

        locations.car_colors = glCheck(glGetUniformLocation(prog, "u_carColors"));
        locations.view_matrix = glCheck(glGetUniformLocation(prog, "u_viewMatrix"));
        locations.model_matrix = glCheck(glGetUniformLocation(prog, "u_modelMatrix"));
        locations.new_model_matrix = glCheck(glGetUniformLocation(prog, "u_newModelMatrix"));
        locations.frame_progress = glCheck(glGetUniformLocation(prog, "u_frameProgress"));
        locations.texture_coords_offset = glCheck(glGetUniformLocation(prog, "u_texCoordsOffset"));
        locations.texture_coords_scale = glCheck(glGetUniformLocation(prog, "u_texCoordsScale"));

        locations.colorizer_matrix = glCheck(glGetUniformLocation(prog, "u_colorizerMatrix"));
        locations.texture_sampler = glCheck(glGetUniformLocation(prog, "u_textureSampler"));
        locations.colorizer_sampler = glCheck(glGetUniformLocation(prog, "u_colorizerSampler")); 
      }

      {
        boundary_shader_program_ = graphics::create_shader_program(shaders::boundary_vertex_shader,
                                                                   shaders::boundary_fragment_shader);

        auto& locations = boundary_locations_;
        auto prog = boundary_shader_program_.get();
        locations.view_matrix = glCheck(glGetUniformLocation(prog, "u_viewMatrix"));
        locations.world_size = glCheck(glGetUniformLocation(prog, "u_worldSize"));
        locations.in_position = glCheck(glGetAttribLocation(prog, "in_position"));
      }
    }

    TrackLayerData::TrackLayerData()
      : vertex_buffer(graphics::create_buffer()),
        index_buffer(graphics::create_buffer())
    {
    }

    void RenderScene::setup_vertex_arrays()
    {
      car_vertex_array_ = graphics::create_vertex_array();
      glCheck(glBindVertexArray(car_vertex_array_.get()));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, car_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, car_index_buffer_.get()));

      glCheck(glEnableVertexAttribArray(car_locations_.in_position));
      glCheck(glEnableVertexAttribArray(car_locations_.in_texCoords));

      glCheck(glVertexAttribPointer(car_locations_.in_position, 2, GL_FLOAT, GL_FALSE, sizeof(CarVertex),
                                    reinterpret_cast<const void*>(offsetof(CarVertex, position))));
      glCheck(glVertexAttribPointer(car_locations_.in_texCoords, 2, GL_FLOAT, GL_FALSE, sizeof(CarVertex),
                                    reinterpret_cast<const void*>(offsetof(CarVertex, texture_coords))));

      boundary_vertex_array_ = graphics::create_vertex_array();
      glCheck(glBindVertexArray(boundary_vertex_array_.get()));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, boundary_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boundary_index_buffer_.get()));

      glCheck(glEnableVertexAttribArray(boundary_locations_.in_position));

      using resources::Vertex;
      glCheck(glVertexAttribPointer(boundary_locations_.in_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, position))));

      glCheck(glBindVertexArray(0));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); 

      update_vaos_ = false;
    }

    void RenderScene::update_track_vertex_arrays()
    {
      for (auto& layer_data : layers_)
      {
        auto& vao = layer_data.second.vertex_array;
        if (vao.get() == 0)
        {
          vao = graphics::create_vertex_array();
          glCheck(glBindVertexArray(vao.get()));

          glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, layer_data.second.index_buffer.get()));
          glCheck(glBindBuffer(GL_ARRAY_BUFFER, layer_data.second.vertex_buffer.get()));

          glCheck(glEnableVertexAttribArray(track_component_locations_.in_position));
          glCheck(glEnableVertexAttribArray(track_component_locations_.in_texCoords));

          using resources::Vertex;
          glCheck(glVertexAttribPointer(track_component_locations_.in_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                        reinterpret_cast<const void*>(offsetof(Vertex, position))));
          glCheck(glVertexAttribPointer(track_component_locations_.in_texCoords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                        reinterpret_cast<const void*>(offsetof(Vertex, texture_coords))));
        }
      }

      glCheck(glBindVertexArray(0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

      update_track_vaos_ = false;
    }

    void RenderScene::setup_entity_buffers()
    {
      car_index_buffer_ = graphics::create_buffer();
      car_vertex_buffer_ = graphics::create_buffer();

      auto make_car_vertex = [](Vector2f pos)
      {
        CarVertex result;
        result.position = pos;
        result.texture_coords = pos;
        return result;
      };

      std::array<CarVertex, 4> vertices;
      vertices[0] = make_car_vertex({ -0.5, -0.5 });
      vertices[1] = make_car_vertex({ -0.5, 0.5 });
      vertices[2] = make_car_vertex({ 0.5, 0.5 });
      vertices[3] = make_car_vertex({ 0.5, -0.5 });      

      std::array<std::uint16_t, 6> indices = { 0, 1, 2, 0, 2, 3 };

      
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, car_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, car_index_buffer_.get()));

      glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW));

      boundary_index_buffer_ = graphics::create_buffer();
      boundary_vertex_buffer_ = graphics::create_buffer();

      using resources::Face;
      using resources::Vertex;

      std::array<std::uint16_t, 6> stencil_indices = { { 0, 1, 2, 1, 2, 3 } };

      std::array<Vertex, 4> stencil_vertices = {};
      stencil_vertices[0].position = { 0.0f, 0.0f };
      stencil_vertices[1].position = { 1.0f, 0.0f };
      stencil_vertices[2].position = { 0.0f, 1.0f };
      stencil_vertices[3].position = { 1.0f, 1.0f };

      auto vertex_buffer_size = stencil_vertices.size() * sizeof(stencil_vertices.front());
      auto element_buffer_size = stencil_indices.size() * sizeof(stencil_indices.front());

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, boundary_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boundary_index_buffer_.get()));

      glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, stencil_vertices.data(), GL_STATIC_DRAW));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, stencil_indices.data(), GL_STATIC_DRAW));

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    void RenderScene::render(const Viewport& view_port, Vector2i screen_size, double frame_progress,
                             const render_callback& post_render) const
    {
      if (update_vaos_)
      {
        const_cast<RenderScene*>(this)->setup_vertex_arrays();
      }

      if (update_track_vaos_)
      {
        const_cast<RenderScene*>(this)->update_track_vertex_arrays();
      }

      glCheck(glDisable(GL_CULL_FACE));
      glCheck(glEnable(GL_BLEND));
      glCheck(glEnable(GL_MULTISAMPLE));
      glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

      glCheck(glDisable(GL_DEPTH_TEST));
      glCheck(glDisable(GL_STENCIL_TEST));
      glCheck(glEnable(GL_TEXTURE_2D));

      auto screen_rect = view_port.screen_rect();
      glCheck(glViewport(screen_rect.left, screen_size.y - screen_rect.bottom(),
                         screen_rect.width, screen_rect.height));

      graphics::scissor_box(screen_rect, screen_size);

      auto world_size = track_scene_.track_size();
      auto view_matrix = compute_view_matrix(view_port, world_size, frame_progress);
      
      glCheck(glStencilMask(0xFF));
      glCheck(glClearColor(background_color_.r, background_color_.g, background_color_.b, background_color_.a));
      glCheck(glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

      glCheck(glUseProgram(boundary_shader_program_.get()));
      
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
      
      glCheck(glUniformMatrix4fv(boundary_locations_.view_matrix, 1, GL_FALSE,
                                 view_matrix.getMatrix()));
      glCheck(glUniform2f(boundary_locations_.world_size, static_cast<float>(world_size.x),
                           static_cast<float>(world_size.y)));

      glBindVertexArray(boundary_vertex_array_.get());
      glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,
                                reinterpret_cast<const void*>(std::uintptr_t(0))));
      
      glCheck(glStencilFunc(GL_EQUAL, 1, 0xFF));
      glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
      
      glCheck(glUseProgram(track_shader_program_.get()));
      glCheck(glUniformMatrix4fv(track_component_locations_.view_matrix, 1, GL_FALSE,
                                 view_matrix.getMatrix()));

      glCheck(glUniform1i(track_component_locations_.texture_sampler, 0));

      std::uint32_t max_level = 0;
      if (!track_components_.empty()) max_level = std::max(track_components_.back().level, max_level);
      if (!drawable_entities_.empty()) max_level = std::max(drawable_entities_.back().level, max_level);


      glCheck(glUseProgram(car_shader_program_.get()));
      glCheck(glUniform1i(car_locations_.texture_sampler, 0));
      glCheck(glUniform1i(car_locations_.colorizer_sampler, 1));
      glCheck(glUniform1f(car_locations_.frame_progress, static_cast<float>(frame_progress)));
      glCheck(glUniformMatrix4fv(car_locations_.view_matrix, 1, GL_FALSE,
                                 view_matrix.getMatrix()));

      auto component_it = track_components_.begin();
      auto entity_it = drawable_entities_.begin();
      for (std::uint32_t level = 0; level <= max_level; ++level)
      {        
        if (component_it != track_components_.end() && level == component_it->level)
        {
          glUseProgram(track_shader_program_.get());
          while (component_it != track_components_.end() && level == component_it->level)
          {            
            const auto& component = *component_it++;

            auto bb = component.bounding_box;
            auto screen_rect = view_matrix.transformRect(sf::FloatRect(bb.left, bb.top, bb.width, bb.height));

            if (screen_rect.left > 1.0 || screen_rect.top > 1.0 ||
                screen_rect.left + screen_rect.width < -1.0 || screen_rect.top + screen_rect.height < -1.0) continue;

            glCheck(glBindVertexArray(component.layer_data->vertex_array.get()));

            glUniform2f(track_component_locations_.min_corner, bb.left - 0.13, bb.top - 0.13);
            glUniform2f(track_component_locations_.max_corner, bb.left + bb.width + 0.13, bb.top + bb.height + 0.13);

            glCheck(glActiveTexture(GL_TEXTURE0));
            glCheck(glBindTexture(GL_TEXTURE_2D, component.texture->get()));

            auto offset = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(component.element_buffer_offset));
            glCheck(glDrawElements(GL_TRIANGLES, component.element_count, GL_UNSIGNED_INT, offset));
          }
        }

        if (entity_it != drawable_entities_.end() && level == entity_it->level)
        {          
          glCheck(glUseProgram(car_shader_program_.get()));
          glCheck(glBindVertexArray(car_vertex_array_.get()));

          while (entity_it != drawable_entities_.end() && level == entity_it->level)
          {
            const auto& e = *entity_it++;

            glCheck(glActiveTexture(GL_TEXTURE0));
            glCheck(glBindTexture(GL_TEXTURE_2D, e.texture->get()));

            glCheck(glActiveTexture(GL_TEXTURE0 + 1));
            glCheck(glBindTexture(GL_TEXTURE_2D, e.colorizer_texture->get()));

            glCheck(glUniformMatrix4fv(car_locations_.model_matrix, 1, GL_FALSE,
                                       e.model_transform.getMatrix()));

            glCheck(glUniformMatrix4fv(car_locations_.new_model_matrix, 1, GL_FALSE,
                                       e.new_model_transform.getMatrix()));           

            glCheck(glUniformMatrix4fv(car_locations_.colorizer_matrix, 1, GL_FALSE,
                                       e.colorizer_transform.getMatrix()));

            glCheck(glUniform3fv(car_locations_.car_colors, 3, e.colors.data()));

            glCheck(glUniform2f(car_locations_.texture_coords_offset, e.texture_coords_offset.x, e.texture_coords_offset.y));
            glCheck(glUniform2f(car_locations_.texture_coords_scale, e.texture_coords_scale.x, e.texture_coords_scale.y));

            glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 
                                   reinterpret_cast<const void*>(std::uintptr_t(0))));
          }
        }
      }

      glCheck(glBindVertexArray(0));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
      glCheck(glUseProgram(0));
      glCheck(glBindTexture(GL_TEXTURE_2D, 0));
      glCheck(glDisable(GL_STENCIL_TEST));

      if (post_render) post_render(view_matrix);

      graphics::disable_scissor_box();
      glCheck(glViewport(0, 0, screen_size.x, screen_size.y));
    }

    void RenderScene::update_entities(const DynamicScene& dynamic_scene)
    {
      // Prepare our local state so that we can easily set the uniform variables for the entities.
      // Loop through all the dynamic entities, and store the required information.

      drawable_entities_.clear();
      for (std::size_t instance_id = 0; instance_id != dynamic_scene.entity_count(); ++instance_id)
      {
        drawable_entities_.push_back(dynamic_scene.entity_info(instance_id));
      }

      std::sort(drawable_entities_.begin(), drawable_entities_.end(),
                [](const DrawableEntity& a, const DrawableEntity& b)
      {
        return a.level < b.level;
      });
    }

    void RenderScene::clear_dynamic_state()
    {
      drawable_entities_.clear();
    }

    void RenderScene::set_background_color(Colorf bg_color)
    {
      background_color_ = bg_color;
    }

    const TrackScene& RenderScene::track_scene() const
    {
      return track_scene_;
    }

    void RenderScene::update_layer_geometry(const resources::TrackLayer* layer)
    {
      const auto& scene_layers = track_scene_.layers();
      for (auto& scene_layer : scene_layers)
      {
        if (layer == nullptr || scene_layer.associated_layer() == layer)
        {
          auto& layer_data = layers_[&scene_layer];
          layer_data.scene_layer = &scene_layer;

          glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, layer_data.index_buffer.get()));
          glCheck(glBindBuffer(GL_ARRAY_BUFFER, layer_data.vertex_buffer.get()));

          track_components_.erase(std::remove_if(track_components_.begin(), track_components_.end(),
                                                 [&](const TrackComponent& component)
          {
            return component.layer_data == &layer_data;
          }), track_components_.end());

          const auto& vertices = scene_layer.vertices();
          auto vertex_data_size = vertices.size() * sizeof(vertices.front());
          layer_data.vertex_buffer_size = next_power_of_two(vertex_data_size);

          glCheck(glBufferData(GL_ARRAY_BUFFER, layer_data.vertex_buffer_size, nullptr, GL_STATIC_DRAW));
          glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_data_size, vertices.data()));

          std::uint32_t index_buffer_size = 0;
          for (auto& region : scene_layer.component_regions())
          {
            for (auto& component : region)
            {
              index_buffer_size += component.faces.size() * sizeof(component.faces.front());
            }
          }

          layer_data.index_buffer_size = next_power_of_two(index_buffer_size);
          glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, layer_data.index_buffer_size, nullptr, GL_STATIC_DRAW));

          std::uint32_t buffer_offset = 0;
          for (auto& region : scene_layer.component_regions())
          {
            for (auto& component : region)
            {
              auto size = component.faces.size() * sizeof(component.faces.front());
              glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, buffer_offset, size, component.faces.data()));

              TrackComponent track_component;
              track_component.layer_data = &layer_data;
              track_component.texture = component.texture;
              track_component.element_buffer_offset = buffer_offset;
              track_component.element_count = component.faces.size() * 3;
              track_component.level = scene_layer.level();
              track_component.z_index = scene_layer.z_index();
              track_component.bounding_box = rect_cast<float>(component.bounding_box);
              track_components_.push_back(track_component);

              buffer_offset += size;
            }
          }
        }
      }

      std::sort(track_components_.begin(), track_components_.end(),
                [](const TrackComponent& a, const TrackComponent& b)
      {
        return std::tie(a.level, a.z_index) < std::tie(b.level, b.z_index);
      });

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

      update_track_vaos_ = true;
    }

    void RenderScene::reorder_track_components()
    {
      for (auto& component : track_components_)
      {
        component.z_index = component.layer_data->scene_layer->z_index();        
      }

      std::sort(track_components_.begin(), track_components_.end(),
                [](const TrackComponent& a, const TrackComponent& b)
      {
        return std::tie(a.level, a.z_index) < std::tie(b.level, b.z_index);
      });
    }

    void RenderScene::reload_track_components()
    {
      using Face = resources::Face;
      track_components_.clear();
      update_layer_geometry(nullptr);
    }

    void RenderScene::add_tile(const resources::TrackLayer* layer,
                               const resources::PlacedTile* tile_expansion, std::size_t tile_count)
    {
      track_scene_.add_tile_geometry(layer, tile_expansion, tile_count);
      update_layer_geometry(layer);
    }

    void RenderScene::rebuild_tile_layer_geometry(const resources::TrackLayer* tile_layer,
                                                  const resources::PlacedTile* tile_expansion, std::size_t tile_count)
    {
      track_scene_.rebuild_tile_layer_geometry(tile_layer, tile_expansion, tile_count);
      update_layer_geometry(tile_layer);
    }

    void RenderScene::rebuild_path_layer_geometry(const resources::TrackLayer* path_layer)
    {
      track_scene_.rebuild_path_layer_geometry(path_layer);
      update_layer_geometry(path_layer);
    }
  }
}
