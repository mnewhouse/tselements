/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "render_scene.hpp"
#include "scene_shaders.hpp"
#include "dynamic_scene.hpp"
#include "view_matrix.hpp"

#include "graphics/gl_scissor_box.hpp"
#include "graphics/gl_check.hpp"

#include "world/world_limits.hpp"

#include <GL/glew.h>
#include <GL/GL.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ts
{
  namespace scene
  {
    using namespace render_scene;

    RenderScene::RenderScene(TrackScene track_scene)
      : track_scene_(std::move(track_scene))
    {
      load_shader_programs();

      load_track_components(track_scene_);

      setup_entity_buffers();

      glCheck(glFinish());
    }

    void RenderScene::load_shader_programs()
    {
      {
        track_shader_program_ = graphics::create_shader_program(shaders::track_vertex_shader,
                                                                shaders::track_fragment_shader);

        auto prog = track_shader_program_.get();

        auto& locations = track_component_uniform_locations_;
        locations.view_matrix = glCheck(glGetUniformLocation(prog, "u_viewMatrix"));
        locations.texture_sampler = glCheck(glGetUniformLocation(prog, "u_textureSampler"));
      }


      {
        car_shader_program_ = graphics::create_shader_program(shaders::car_vertex_shader,
                                                              shaders::car_fragment_shader);

        auto prog = car_shader_program_.get();
        auto& locations = car_uniform_locations_;
        locations.car_colors = glCheck(glGetUniformLocation(prog, "u_carColors"));
        locations.view_matrix = glCheck(glGetUniformLocation(prog, "u_viewMatrix"));
        locations.model_matrix = glCheck(glGetUniformLocation(prog, "u_modelMatrix"));
        locations.new_model_matrix = glCheck(glGetUniformLocation(prog, "u_newModelMatrix"));
        locations.frame_progress = glCheck(glGetUniformLocation(prog, "u_frameProgress"));
        locations.colorizer_matrix = glCheck(glGetUniformLocation(prog, "u_colorizerMatrix"));
        locations.texture_sampler = glCheck(glGetUniformLocation(prog, "u_textureSampler"));
        locations.colorizer_sampler = glCheck(glGetUniformLocation(prog, "u_colorizerSampler"));
      }
      
      {
        boundary_shader_program_ = graphics::create_shader_program(shaders::boundary_vertex_shader,
                                                                   shaders::boundary_fragment_shader);

        auto& locations = boundary_uniform_locations_;
        auto prog = boundary_shader_program_.get();
        locations.view_matrix = glCheck(glGetUniformLocation(prog, "u_viewMatrix"));
        locations.world_size = glCheck(glGetUniformLocation(prog, "u_worldSize"));
      }
    }

    void RenderScene::setup_entity_buffers()
    {
      car_index_buffer_ = graphics::create_buffer();
      car_vertex_buffer_ = graphics::create_buffer();

      const std::uint32_t max_index = world::limits::max_car_count;

      std::vector<std::uint32_t> index_buffer;
      index_buffer.reserve(max_index * 6);
      for (std::uint32_t index = 0; index < max_index * 4; index += 4)
      {
        index_buffer.insert(index_buffer.end(), { index, index + 1, index + 2, index + 1, index + 2, index + 3 });
      }
      
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, car_index_buffer_.get()));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer.size() * sizeof(index_buffer.front()),
                           index_buffer.data(), GL_STATIC_DRAW));

    }

    void RenderScene::load_track_components(const TrackScene& track_scene)
    {
      boundary_index_buffer_ = graphics::create_buffer();
      boundary_vertex_buffer_ = graphics::create_buffer();

      using resources::Face;
      using resources::Vertex;

      std::array<Face, 2> stencil_faces =
      { {
        { 0, 1, 2 },
        { 1, 2, 3 }
      } };

      std::array<Vertex, 4> stencil_vertices = {};
      stencil_vertices[0].position = { 0.0f, 0.0f };
      stencil_vertices[1].position = { 1.0f, 0.0f };
      stencil_vertices[2].position = { 0.0f, 1.0f };
      stencil_vertices[3].position = { 1.0f, 1.0f };

      auto vertex_buffer_size = stencil_vertices.size() * sizeof(stencil_vertices.front());
      auto element_buffer_size = stencil_faces.size() * sizeof(stencil_faces.front());

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, boundary_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boundary_index_buffer_.get()));

      glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, stencil_vertices.data(), GL_STATIC_DRAW));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, stencil_faces.data(), GL_STATIC_DRAW));
      
      // Create all the necessary layer data and populate the buffers.
      for (const auto& scene_layer : track_scene.layers())
      {
        auto track_layer = scene_layer.associated_layer();
        auto& layer_data = layers_[track_layer];

        const auto& vertices = scene_layer.vertices();
        const auto& faces = scene_layer.faces();

        auto index_data_size = faces.size() * sizeof(faces.front());
        auto vertex_data_size = vertices.size() * sizeof(vertices.front());

        layer_data.index_buffer = graphics::create_buffer();
        layer_data.vertex_buffer = graphics::create_buffer();

        layer_data.index_buffer_size = graphics::next_power_of_two(index_data_size);
        layer_data.vertex_buffer_size = graphics::next_power_of_two(vertex_data_size);

        glCheck(glBindBuffer(GL_ARRAY_BUFFER, layer_data.vertex_buffer.get()));
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, layer_data.index_buffer.get()));

        glCheck(glBufferData(GL_ARRAY_BUFFER, layer_data.vertex_buffer_size, nullptr, GL_STATIC_DRAW));
        glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, layer_data.index_buffer_size, nullptr, GL_STATIC_DRAW));

        glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_data_size, vertices.data()));
        glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, index_data_size, faces.data()));
      }

      reload_track_components();
    }

    void RenderScene::render(const Viewport& view_port, Vector2i screen_size, double frame_progress,
                             const render_callback& post_render) const
    {
      glCheck(glDisable(GL_CULL_FACE));
      glCheck(glEnable(GL_BLEND));
      glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

      glCheck(glDisable(GL_DEPTH_TEST));
      glCheck(glEnable(GL_STENCIL_TEST));
      glCheck(glEnable(GL_TEXTURE_2D));

      auto screen_rect = view_port.screen_rect();

      glCheck(glViewport(screen_rect.left, screen_size.y - screen_rect.bottom(),
                         screen_rect.width, screen_rect.height));

      graphics::scissor_box(screen_size, screen_rect);

      glCheck(glStencilMask(0xFF));
      glCheck(glClearColor(background_color_.r, background_color_.g, background_color_.b, background_color_.a));
      glCheck(glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

      auto world_size = track_scene_.track_size();
      auto view_matrix = compute_view_matrix(view_port, world_size, frame_progress);
      
      glCheck(glUseProgram(boundary_shader_program_.get()));

      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

      glCheck(glUniformMatrix4fv(boundary_uniform_locations_.view_matrix, 1, GL_FALSE,
                                 glm::value_ptr(view_matrix)));
      glCheck(glUniform2f(boundary_uniform_locations_.world_size, static_cast<float>(world_size.x),
                          static_cast<float>(world_size.y)));

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, boundary_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boundary_index_buffer_.get()));

      glCheck(glEnableVertexAttribArray(0));
      glCheck(glEnableVertexAttribArray(1));

      using resources::Vertex;
      glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, position))));
      glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                    reinterpret_cast<const void*>(offsetof(Vertex, texture_coords))));

      glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 
                             reinterpret_cast<const void*>(std::uintptr_t(0))));

      glCheck(glStencilFunc(GL_EQUAL, 1, 0xFF));
      glCheck(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));

      glCheck(glUseProgram(track_shader_program_.get()));

      glCheck(glUniformMatrix4fv(track_component_uniform_locations_.view_matrix, 1, GL_FALSE,
                                 glm::value_ptr(view_matrix)));

      glCheck(glUniform1i(track_component_uniform_locations_.texture_sampler, 0));

      glCheck(glActiveTexture(GL_TEXTURE0));
      for (const auto& component : track_components_)
      {
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, component.layer_data->vertex_buffer.get()));
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, component.layer_data->index_buffer.get()));

        glCheck(glEnableVertexAttribArray(0));
        glCheck(glEnableVertexAttribArray(1));

        using resources::Vertex;
        glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                      reinterpret_cast<const void*>(offsetof(Vertex, position))));
        glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                      reinterpret_cast<const void*>(offsetof(Vertex, texture_coords))));

        glCheck(glBindTexture(GL_TEXTURE_2D, component.texture->get()));

        auto offset = reinterpret_cast<const void*>(static_cast<std::uintptr_t>(component.element_buffer_offset));
        glCheck(glDrawElements(GL_TRIANGLES, component.element_count, GL_UNSIGNED_INT, offset));
      }

      glCheck(glUseProgram(car_shader_program_.get()));
      glCheck(glUniform1i(car_uniform_locations_.texture_sampler, 0));
      glCheck(glUniform1i(car_uniform_locations_.colorizer_sampler, 1));
      glCheck(glUniform1f(car_uniform_locations_.frame_progress, static_cast<float>(frame_progress)));

      glCheck(glUniformMatrix4fv(car_uniform_locations_.view_matrix, 1, GL_FALSE,
                                 glm::value_ptr(view_matrix)));

      glCheck(glEnableVertexAttribArray(2));

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, car_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, car_index_buffer_.get()));

      using CarVertex = DrawableEntity::Vertex;
      glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CarVertex),
                                    reinterpret_cast<const void*>(offsetof(CarVertex, position))));
      glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CarVertex),
                                    reinterpret_cast<const void*>(offsetof(CarVertex, texture_coords))));
      glCheck(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CarVertex),
                                    reinterpret_cast<const void*>(offsetof(CarVertex, colorizer_coords))));

      std::uintptr_t element_buffer_offset = 0;
      for (const auto& e : drawable_entities_)
      {
        glCheck(glActiveTexture(GL_TEXTURE0));
        glCheck(glBindTexture(GL_TEXTURE_2D, e.texture->get()));

        // Bind colorizer texture
        glCheck(glUniform4fv(car_uniform_locations_.car_colors, 3, e.colors.data()));
        glCheck(glUniformMatrix4fv(car_uniform_locations_.model_matrix, 1, GL_FALSE,
                                   glm::value_ptr(e.model_matrix)));

        glCheck(glUniformMatrix4fv(car_uniform_locations_.new_model_matrix, 1, GL_FALSE,
                                   glm::value_ptr(e.new_model_matrix)));
        
        glCheck(glUniformMatrix4fv(car_uniform_locations_.colorizer_matrix, 1, GL_FALSE,
                                   glm::value_ptr(e.colorizer_matrix)));

        glCheck(glUniform4fv(car_uniform_locations_.car_colors, 3, e.colors.data()));

        glCheck(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 
                               reinterpret_cast<const void*>(element_buffer_offset)));

        element_buffer_offset += sizeof(std::uint32_t) * 6;
      }

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
      glCheck(glUseProgram(0));
      glCheck(glBindTexture(GL_TEXTURE_2D, 0));
      glCheck(glDisable(GL_STENCIL_TEST));

      if (post_render) post_render(view_matrix);

      graphics::disable_scissor_box();
      glCheck(glViewport(0, 0, screen_size.x, screen_size.y));
    }

    void RenderScene::update_entities(const DynamicScene& dynamic_scene, std::uint32_t frame_duration)
    {
      // Prepare our local state so that we can easily set the uniform variables for the entities.
      // Loop through all the dynamic entities, and store the required information.

      drawable_entities_.clear();
      car_vertex_buffer_cache_.clear();

      for (std::size_t instance_id = 0; instance_id != dynamic_scene.entity_count(); ++instance_id)
      {
        auto drawable_entity = dynamic_scene.entity_info(instance_id);

        drawable_entities_.emplace_back();

        auto& entity_info = drawable_entities_.back();
        entity_info.texture = drawable_entity.texture;
        entity_info.model_matrix = drawable_entity.model_matrix;
        entity_info.new_model_matrix = drawable_entity.new_model_matrix;
        entity_info.colorizer_matrix = drawable_entity.colorizer_matrix;
        
        std::uint32_t color_index = 0;
        for (auto& color : drawable_entity.colors)
        {
          entity_info.colors[color_index++] = color.r;
          entity_info.colors[color_index++] = color.g;
          entity_info.colors[color_index++] = color.b;
          entity_info.colors[color_index++] = color.a;
        }

        car_vertex_buffer_cache_.insert(car_vertex_buffer_cache_.end(), 
                                        drawable_entity.vertices.begin(), drawable_entity.vertices.end());
      }

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, car_vertex_buffer_.get()));
      glCheck(glBufferData(GL_ARRAY_BUFFER, car_vertex_buffer_cache_.size() * sizeof(car_vertex_buffer_cache_.front()),
                           car_vertex_buffer_cache_.data(), GL_STREAM_DRAW));
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

    void RenderScene::update_layer_geometry(const resources::TrackLayer* layer, 
                                            render_scene::TrackLayerData& layer_data,
                                            const TrackScene::GeometryUpdate& geometry_update)
    {
      if (auto scene_layer = track_scene_.find_layer(layer))
      {
        if (geometry_update.face_begin != geometry_update.face_end)
        {
          glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, layer_data.index_buffer.get()));

          const auto& faces = scene_layer->faces();
          auto data_size = faces.size() * sizeof(faces.front());

          if (data_size > layer_data.index_buffer_size)
          {  
            layer_data.index_buffer_size = graphics::next_power_of_two(data_size);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, layer_data.index_buffer_size, nullptr, GL_STATIC_DRAW);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, data_size, faces.data());
          }

          else
          {
            auto range_start = geometry_update.face_begin * sizeof(faces.front());
            auto range_size = geometry_update.face_end * sizeof(faces.front()) - range_start;
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, range_start, range_size,
                            faces.data() + geometry_update.face_begin);
          }

          glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        }


        if (geometry_update.vertex_begin != geometry_update.vertex_end)
        {
          glCheck(glBindBuffer(GL_ARRAY_BUFFER, layer_data.vertex_buffer.get()));

          const auto& vertices = scene_layer->vertices();
          auto data_size = vertices.size() * sizeof(vertices.front());

          if (data_size > layer_data.vertex_buffer_size)
          {
            layer_data.vertex_buffer_size = graphics::next_power_of_two(data_size);
            glBufferData(GL_ARRAY_BUFFER, layer_data.vertex_buffer_size, nullptr, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, data_size, vertices.data());
          }

          else
          {
            auto range_start = geometry_update.vertex_begin * sizeof(vertices.front());
            auto range_size = geometry_update.vertex_end * sizeof(vertices.front()) - range_start;
            glBufferSubData(GL_ARRAY_BUFFER, range_start, range_size,
                            vertices.data() + geometry_update.vertex_begin);
          }

          glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }
      }
    }

    void RenderScene::reload_track_components()
    {
      using Face = resources::Face;

      // Now, store the component information so that we can display the scene in the proper order.
      for (const auto& component : track_scene_.components())
      {
        auto& layer_data = layers_[component.track_layer];

        // Loop through the layer's components and store the information we need.
        TrackComponent track_component;
        track_component.element_buffer_offset = component.face_index * sizeof(Face);
        track_component.element_count = component.face_count * 3;
        track_component.level = component.level;
        track_component.texture = component.texture;
        track_component.layer_data = &layer_data;

        track_components_.push_back(track_component);
      }
    }
    
    void RenderScene::add_tile(const resources::TrackLayer* layer, std::uint32_t tile_index,
                               const resources::PlacedTile* tile_expansion, std::size_t sub_tile_count)
    {
      auto layer_it = layers_.find(layer);
      if (layer_it != layers_.end())
      {
        auto geometry_update = track_scene_.add_tile_geometry(layer, tile_index, tile_expansion, sub_tile_count);

        update_layer_geometry(layer, layer_it->second, geometry_update);

        track_scene_.reload_components();
        reload_track_components();
      }     
    }

    void RenderScene::remove_tile(const resources::TrackLayer* layer, std::uint32_t tile_index)
    {
      auto layer_it = layers_.find(layer);
      if (layer_it != layers_.end())
      {
        auto geometry_update = track_scene_.remove_item_geometry(layer, tile_index);

        update_layer_geometry(layer, layer_it->second, geometry_update);

        track_scene_.reload_components();
        reload_track_components();
      }
    }
  }
}
