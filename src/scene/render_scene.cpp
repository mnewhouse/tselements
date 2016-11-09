/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "render_scene.hpp"
#include "scene_shaders.hpp"
#include "dynamic_scene.hpp"

#include "graphics/gl_scissor_box.hpp"
#include "graphics/gl_check.hpp"

#include "world/world_limits.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/GL.h>

namespace ts
{
  namespace scene
  {
    namespace detail
    {
      auto compute_view_matrix(const Viewport& viewport, Vector2u world_size,
                                    Vector2u screen_size, double frame_progress)
      {
        const auto& camera = viewport.camera();

        auto zoom_level = static_cast<float>(camera.zoom_level());
        auto position = vector2_cast<float>(camera.position());
        auto rotation = static_cast<float>(camera.rotation().radians());

        auto center = compute_camera_center(camera, vector2_cast<double>(world_size),
                                            vector2_cast<double>(screen_size), frame_progress);

        glm::vec3 scale(2.0f * zoom_level / screen_size.x, -2.0f * zoom_level / screen_size.y, 0.0f);
        glm::vec3 translation(-center.x, -center.y, 0.0f);

        auto matrix = glm::scale(glm::mat4(), scale);
        matrix = glm::rotate(matrix, -rotation, { 0.0f, 0.0f, 1.0f });
        matrix = glm::translate(matrix, translation);

        return matrix;
      }
    }

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

        auto& locations = track_component_uniform_locations_;
        locations.view_matrix = glCheck(glGetUniformLocation(track_shader_program_.get(), "u_viewMatrix"));
        locations.texture_sampler = glCheck(glGetAttribLocation(track_shader_program_.get(), "u_textureSampler"));
      }


      {
        car_shader_program_ = graphics::create_shader_program(shaders::car_vertex_shader,
                                                              shaders::car_fragment_shader);

        auto& locations = car_uniform_locations_;
        locations.car_colors = glCheck(glGetUniformLocation(car_shader_program_.get(), "u_carColors"));
        locations.view_matrix = glCheck(glGetUniformLocation(car_shader_program_.get(), "u_viewMatrix"));
        locations.model_matrix = glCheck(glGetUniformLocation(car_shader_program_.get(), "u_modelMatrix"));
        locations.new_model_matrix = glCheck(glGetUniformLocation(car_shader_program_.get(), "u_newModelMatrix"));
        locations.colorizer_matrix = glCheck(glGetUniformLocation(car_shader_program_.get(), "u_colorizerMatrix"));
        locations.frame_progress = glCheck(glGetUniformLocation(car_shader_program_.get(), "u_frameProgress"));
        locations.texture_sampler = glCheck(glGetUniformLocation(car_shader_program_.get(), "u_texSampler"));
        locations.colorizer_sampler = glCheck(glGetUniformLocation(car_shader_program_.get(), "u_colorizerSampler"));
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
      track_component_vertex_buffer_ = graphics::create_buffer();
      track_component_element_buffer_ = graphics::create_buffer();

      std::size_t vertex_buffer_size = 0;
      std::size_t element_buffer_size = 0;

      // Find the size of the buffers that we need
      for (const auto& scene_layer : track_scene.active_layers())
      {
        const auto& vertices = scene_layer.vertices();
        const auto& faces = scene_layer.faces();

        vertex_buffer_size += vertices.size() * sizeof(vertices.front());
        element_buffer_size += faces.size() * sizeof(faces.front());
      }

      // Reserve some more space in order to be able to move components around,
      // then round up to the nearest power of two.
      vertex_buffer_size = graphics::next_power_of_two(vertex_buffer_size + vertex_buffer_size / 2);
      element_buffer_size = graphics::next_power_of_two(element_buffer_size + element_buffer_size / 2);
      
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, track_component_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, track_component_element_buffer_.get()));

      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, nullptr, GL_STATIC_DRAW));
      glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, nullptr, GL_STATIC_DRAW));

      std::size_t vertex_buffer_offset = 0;
      std::size_t element_buffer_offset = 0;
      std::uint32_t vertex_index = 0;

      // Now, populate the buffers with the layers' vertices and faces.
      // Also, store the component information so that we can render it properly.
      for (const auto& scene_layer : track_scene.active_layers())
      {
        const auto& vertices = scene_layer.vertices();
        auto faces = scene_layer.faces();

        for (auto& face : faces)
        {
          face.first_index += vertex_index;
          face.second_index += vertex_index;
          face.third_index += vertex_index;
        }

        auto vertex_range_size = vertices.size() * sizeof(vertices.front());
        auto element_range_size = faces.size() * sizeof(faces.front());

        glCheck(glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, vertex_range_size, vertices.data()));
        glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_offset, element_range_size, faces.data()));

        // Loop through the layer's components and store the information we need.
        for (const auto& component : scene_layer.components())
        {
          TrackComponent track_component;
          track_component.element_buffer_offset = element_buffer_offset + component.face_index * sizeof(faces.front());
          track_component.element_count = component.face_count * 3;
          track_component.level = scene_layer.level();
          track_component.texture = component.texture;
          track_components_.push_back(track_component);
        }

        vertex_buffer_offset += vertex_range_size;
        element_buffer_offset += element_range_size;

        vertex_index += vertices.size();
      }
    }

    void RenderScene::render(const Viewport& view_port, Vector2u screen_size, double frame_progress) const
    {
      glCheck(glDisable(GL_CULL_FACE));
      glCheck(glEnable(GL_BLEND));
      glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

      glCheck(glDisable(GL_DEPTH_TEST));
      glCheck(glDisable(GL_STENCIL_TEST));

      glCheck(glEnable(GL_TEXTURE_2D));

      graphics::scissor_box(screen_size, view_port.screen_rect());

      glCheck(glBindBuffer(GL_ARRAY_BUFFER, track_component_vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, track_component_element_buffer_.get()));

      glCheck(glUseProgram(track_shader_program_.get()));

      auto view_matrix = detail::compute_view_matrix(view_port, track_scene_.track_size(), screen_size, frame_progress);

      glCheck(glUniformMatrix4fv(track_component_uniform_locations_.view_matrix, 1, GL_FALSE,
                                 glm::value_ptr(view_matrix)));

      glCheck(glUniform1i(track_component_uniform_locations_.texture_sampler, 0));

      glCheck(glEnableVertexAttribArray(0));
      glCheck(glEnableVertexAttribArray(1));

      using resources::TrackVertex;
      glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TrackVertex),
                                    reinterpret_cast<const void*>(offsetof(TrackVertex, position))));
      glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TrackVertex),
                                    reinterpret_cast<const void*>(offsetof(TrackVertex, texture_coords))));

      for (const auto& component : track_components_)
      {
        glCheck(glActiveTexture(GL_TEXTURE0));
        glCheck(glBindTexture(GL_TEXTURE_2D, component.texture->get()));

        glCheck(glDrawElements(GL_TRIANGLES, component.element_count, GL_UNSIGNED_INT,
                               reinterpret_cast<const void*>(component.element_buffer_offset)));

      }

      glUseProgram(car_shader_program_.get());
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

      graphics::disable_scissor_box();
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
                           car_vertex_buffer_cache_.data(), GL_STATIC_DRAW));
    }
  }
}
