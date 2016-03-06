/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "scene_renderer.hpp"
#include "track_scene.hpp"
#include "dynamic_scene.hpp"
#include "particle_generator.hpp"
#include "shader_code.hpp"
#include "viewport.hpp"

#include "graphics/shader.hpp"
#include "graphics/vertex_array.hpp"

#include "world/world_limits.hpp"

#include "utility/vertex.hpp"
#include "utility/transform.hpp"

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <tuple>
#include <algorithm>
#include <cstdint>
#include <array>
#include <iostream>

namespace ts
{
  namespace scene
  {
    struct EntityVertex
    {
      Vector2<float> position;
      Vector2<float> frame_offset;

      Vector2<float> texture_coords;
      Vector2<float> colorizer_coords;

      Rect<float> colorizer_bounds;

      Colorb primary_color;
      Colorb secondary_color;
      Colorb tertiary_color;

      float padding;
    };

    namespace detail
    {
      static std::size_t next_power_of_two(std::size_t n)
      {
        std::size_t result = 1; 
        while (result < n) result <<= 1;

        return result;
      }

      const std::size_t vertices_per_entity = 4;
      const std::size_t indices_per_entity = 6;

      const std::size_t entity_buffer_size = next_power_of_two(
        world::limits::max_car_count * sizeof(EntityVertex) * vertices_per_entity +
        world::limits::max_car_count * sizeof(GLuint) * indices_per_entity
      );
    }

    namespace detail
    {
      enum class ShaderType
      {
        None,
        Car      
      };

      struct DrawableEntityGroup
      {
        std::size_t element_index;
        std::size_t element_count;

        const graphics::Texture* texture;
        ShaderType shader_type;
        std::uint32_t level;
      };

      static ShaderType entity_type_to_shader_type(world::EntityType entity_type);
      static void generate_entity_indices(GLuint* index_ptr, GLuint entity_count, GLuint vertex_index);
      static void generate_entity_vertices(const DrawableEntity& drawable_entity, EntityVertex* vertices,
                                           Vector2<float> frame_movement);
    }

    struct SceneRenderer::Impl
    {
      explicit Impl(const TrackScene* track_scene_, const DynamicScene* dynamic_scene_,
                    const ParticleGenerator* particle_generator_)
        : track_scene(track_scene_),
          dynamic_scene(dynamic_scene_),
          particle_generator(particle_generator_)
      {}

      void initialize_track_shaders();
      void initialize_entity_shaders();
      void initialize_particle_shaders();
      
      void initialize_track_buffers();
      void initialize_entity_buffers();
      void initialize_particle_buffers();

      std::size_t calculate_particle_buffer_size() const;   

      const TrackScene* track_scene;
      const DynamicScene* dynamic_scene;
      const ParticleGenerator* particle_generator;

      bool rendering_initialized_ = false;
      graphics::Shader track_vertex_shader;
      graphics::Shader track_fragment_shader;
      graphics::Sampler texture_sampler;
      graphics::Sampler color_sampler;
      graphics::ShaderProgram track_shader_program;
      graphics::VertexArray track_vertex_array;
      graphics::Buffer track_vertex_buffer;
      GLint track_view_matrix_location = 0;      

      graphics::Shader car_vertex_shader;
      graphics::Shader car_fragment_shader;
      graphics::ShaderProgram car_shader_program;
      GLint car_view_matrix_location = 0;
      GLint car_frame_progress_location = 0;

      graphics::Shader shadow_vertex_shader;
      graphics::Shader shadow_fragment_shader;
      graphics::ShaderProgram shadow_shader_program;
      GLint shadow_view_matrix_location = 0;
      GLint shadow_color_location = 0;
      GLint shadow_frame_progress_location = 0;

      graphics::Shader particle_vertex_shader;
      graphics::Shader particle_fragment_shader;
      graphics::ShaderProgram particle_shader_program;
      GLint particle_view_matrix_location = 0;
      graphics::Buffer particle_vertex_buffer;
      graphics::VertexArray particle_vertex_array;
      std::size_t particle_buffer_size = 0;
      std::size_t particle_buffer_offset = 0;
      std::size_t particle_buffer_draw_offset = 0;
      std::size_t particle_buffer_range_size = 0;
      std::size_t particle_index_count = 0;
      std::size_t particle_vertex_index = 0;

      // Dynamic entities are grouped by shader/texture combination, so that we can
      // draw them all at the same time, given only one type of entity.
      graphics::Buffer entity_vertex_buffer;
      graphics::VertexArray entity_vertex_array;
      std::size_t entity_buffer_offset = 0;
      std::size_t entity_buffer_draw_offset = 0;
      std::size_t entity_buffer_range_size = 0;
      std::size_t entity_vertex_index = 0;
      
      std::vector<detail::DrawableEntityGroup> entity_groups;
      std::vector<DrawableEntity> drawable_cache;
    };

    void SceneRenderer::Impl::initialize_track_shaders()
    {
      using graphics::Shader;
      using graphics::ShaderProgram;

      track_vertex_shader = Shader(glCreateShader(GL_VERTEX_SHADER));
      track_fragment_shader = Shader(glCreateShader(GL_FRAGMENT_SHADER));
      track_shader_program = ShaderProgram(glCreateProgram());

      graphics::compile_shader(track_vertex_shader, shaders::track_vertex_shader);
      graphics::compile_shader(track_fragment_shader, shaders::track_fragment_shader);

      glAttachShader(track_shader_program.get(), track_vertex_shader.get());
      glAttachShader(track_shader_program.get(), track_fragment_shader.get()); 
      
      graphics::link_shader_program(track_shader_program);

      // Only use texture unit 0 for the renderer
      glUseProgram(track_shader_program.get());
      glUniform1i(glGetUniformLocation(track_shader_program.get(), "texSampler"), 0);

      auto world_size = vector2_cast<float>(track_scene->track_size());
      glUniform2f(glGetUniformLocation(track_shader_program.get(), "worldSize"), world_size.x, world_size.y);

      track_view_matrix_location = glGetUniformLocation(track_shader_program.get(), "viewMat");

      {
        using graphics::Sampler;
        using graphics::Buffer;

        GLuint sampler = 0;
        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        texture_sampler = Sampler(sampler);

        GLuint color_sampler_id = 0;
        glGenSamplers(1, &color_sampler_id);
        glSamplerParameteri(color_sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(color_sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        color_sampler = Sampler(color_sampler_id);

        GLuint buffer = 0;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        track_vertex_buffer = Buffer(buffer);

        using Vertex = TrackScene::vertex_type;
        std::size_t buffer_size = 0;
        for (const auto& component : track_scene->components())
        {
          buffer_size += component.vertex_count;
        }

        buffer_size = detail::next_power_of_two(buffer_size * sizeof(Vertex));
        glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_STATIC_DRAW);

        std::size_t offset = 0;
        for (const auto& component : track_scene->components())
        {
          auto size = component.vertex_count * sizeof(Vertex);
          glBufferSubData(GL_ARRAY_BUFFER, offset, size, component.vertices);
          offset += size;
        }
      }

      glUseProgram(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void SceneRenderer::Impl::initialize_track_buffers()
    {
      using Vertex = TrackScene::vertex_type;
      using graphics::VertexArray;

      GLuint vao = 0;
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      track_vertex_array = VertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, track_vertex_buffer.get());

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            reinterpret_cast<const GLvoid*>(offsetof(Vertex, position)));
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                            reinterpret_cast<const GLvoid*>(offsetof(Vertex, texture_coords)));

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void SceneRenderer::Impl::initialize_entity_shaders()
    {
      using graphics::Shader;
      using graphics::ShaderProgram;

      car_vertex_shader = Shader(glCreateShader(GL_VERTEX_SHADER));
      car_fragment_shader = Shader(glCreateShader(GL_FRAGMENT_SHADER));
      car_shader_program = ShaderProgram(glCreateProgram());

      graphics::compile_shader(car_vertex_shader, shaders::car_vertex_shader);
      graphics::compile_shader(car_fragment_shader, shaders::car_fragment_shader);

      glAttachShader(car_shader_program.get(), car_vertex_shader.get());
      glAttachShader(car_shader_program.get(), car_fragment_shader.get());

      graphics::link_shader_program(car_shader_program);

      glUseProgram(car_shader_program.get());
      glUniform1i(glGetUniformLocation(car_shader_program.get(), "texSampler"), 0);
      glUniform1i(glGetUniformLocation(car_shader_program.get(), "colorSampler"), 1);

      auto world_size = vector2_cast<float>(track_scene->track_size());
      glUniform2f(glGetUniformLocation(car_shader_program.get(), "worldSize"), world_size.x, world_size.y);

      car_frame_progress_location = glGetUniformLocation(car_shader_program.get(), "frameProgress");
      car_view_matrix_location = glGetUniformLocation(car_shader_program.get(), "viewMat");

      shadow_vertex_shader = Shader(glCreateShader(GL_VERTEX_SHADER));
      shadow_fragment_shader = Shader(glCreateShader(GL_FRAGMENT_SHADER));
      shadow_shader_program = ShaderProgram(glCreateProgram());

      graphics::compile_shader(shadow_vertex_shader, shaders::shadow_vertex_shader);
      graphics::compile_shader(shadow_fragment_shader, shaders::shadow_fragment_shader);

      glAttachShader(shadow_shader_program.get(), shadow_vertex_shader.get());
      glAttachShader(shadow_shader_program.get(), shadow_fragment_shader.get());

      graphics::link_shader_program(shadow_shader_program);

      glUseProgram(shadow_shader_program.get());
      glUniform1i(glGetUniformLocation(shadow_shader_program.get(), "texSampler"), 0);
      glUniform2f(glGetUniformLocation(shadow_shader_program.get(), "worldSize"), world_size.x, world_size.y);

      shadow_view_matrix_location = glGetUniformLocation(shadow_shader_program.get(), "viewMat");
      shadow_color_location = glGetUniformLocation(shadow_shader_program.get(), "shadowColor");
      shadow_frame_progress_location = glGetUniformLocation(shadow_shader_program.get(), "frameProgress");

      {
        GLuint vertex_buffer = 0;
        glGenBuffers(1, &vertex_buffer);
        entity_vertex_buffer = graphics::Buffer(vertex_buffer);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, detail::entity_buffer_size, nullptr, GL_DYNAMIC_DRAW);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glUseProgram(0);
    }

    void SceneRenderer::Impl::initialize_entity_buffers()
    {
      GLuint vao;
      glGenVertexArrays(1, &vao);
      entity_vertex_array = graphics::VertexArray(vao);
      glBindVertexArray(vao);

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glEnableVertexAttribArray(3);
      glEnableVertexAttribArray(4);
      glEnableVertexAttribArray(5);
      glEnableVertexAttribArray(6);
      glEnableVertexAttribArray(7);

      glBindBuffer(GL_ARRAY_BUFFER, entity_vertex_buffer.get());

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(EntityVertex, position)));

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(EntityVertex, texture_coords)));      

      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(EntityVertex, frame_offset)));

      glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(EntityVertex, colorizer_coords)));

      glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(EntityVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(EntityVertex, colorizer_bounds)));

      glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(EntityVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(EntityVertex, primary_color)));

      glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(EntityVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(EntityVertex, secondary_color)));

      glVertexAttribPointer(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(EntityVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(EntityVertex, tertiary_color)));

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entity_vertex_buffer.get());

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void SceneRenderer::Impl::initialize_particle_shaders()
    {
      using graphics::Shader;
      using graphics::ShaderProgram;

      particle_vertex_shader = Shader(glCreateShader(GL_VERTEX_SHADER));
      particle_fragment_shader = Shader(glCreateShader(GL_FRAGMENT_SHADER));
      particle_shader_program = ShaderProgram(glCreateProgram());

      graphics::compile_shader(particle_vertex_shader, shaders::particle_vertex_shader);
      graphics::compile_shader(particle_fragment_shader, shaders::particle_fragment_shader);

      glAttachShader(particle_shader_program.get(), particle_vertex_shader.get());
      glAttachShader(particle_shader_program.get(), particle_fragment_shader.get());

      graphics::link_shader_program(particle_shader_program);

      glUseProgram(particle_shader_program.get());
      particle_view_matrix_location = glGetUniformLocation(particle_shader_program.get(), "viewMat"); 

      {
        using graphics::Buffer;

        GLuint buffer;
        glGenBuffers(1, &buffer);
        particle_vertex_buffer = Buffer(buffer);

        particle_buffer_size = calculate_particle_buffer_size();

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, particle_buffer_size, nullptr, GL_DYNAMIC_DRAW);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glUseProgram(0);
    }

    void SceneRenderer::Impl::initialize_particle_buffers()
    {
      GLuint vao;
      glGenVertexArrays(1, &vao);
      particle_vertex_array = graphics::VertexArray(vao);
      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, particle_vertex_buffer.get());

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);      

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(ParticleVertex, position)));

      glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ParticleVertex),
                            reinterpret_cast<const GLvoid*>(offsetof(ParticleVertex, color)));
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particle_vertex_buffer.get());

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    std::size_t SceneRenderer::Impl::calculate_particle_buffer_size() const
    {
      auto vertex_count = particle_generator->max_particles() * particle_generator->vertices_per_particle();
      auto index_count = particle_generator->max_particles() * particle_generator->indices_per_particle();

      auto buffer_size = vertex_count * sizeof(ParticleVertex) + index_count * sizeof(GLuint);
      if (auto mod = buffer_size % sizeof(ParticleVertex))
      {
        buffer_size -= mod;
        buffer_size += sizeof(ParticleVertex);
      }

      return detail::next_power_of_two(buffer_size * 3);
    }

    SceneRenderer::SceneRenderer()
    {
    }

    SceneRenderer::SceneRenderer(const TrackScene* track_scene, const DynamicScene* dynamic_scene, 
                                 const ParticleGenerator* particle_generator)
      : impl_(std::make_unique<Impl>(track_scene, dynamic_scene, particle_generator))
    {
      // Make sure we allocate enough to not have to allocate during gameplay
      impl_->initialize_track_shaders();
      impl_->initialize_entity_shaders();
      impl_->initialize_particle_shaders();
    }

    SceneRenderer::~SceneRenderer()
    {
    }

    SceneRenderer::SceneRenderer(SceneRenderer&& other) = default;
    SceneRenderer& SceneRenderer::operator=(SceneRenderer&& other) = default;

    namespace detail
    {
      glm::mat4 compute_matrix(const Viewport& viewport, Vector2u world_size,
                               Vector2u screen_size, double frame_progress)
      {
        const auto& camera = viewport.camera();
        auto screen_rect = rect_cast<float>(viewport.screen_rect());
        auto zoom_level = static_cast<float>(camera.zoom_level());
        auto position = vector2_cast<float>(camera.position());
        auto rotation = static_cast<float>(camera.rotation().radians());

        auto center = compute_camera_center(camera, world_size, screen_size, frame_progress);

        glm::vec3 scale(2.0f * zoom_level / screen_size.x, -2.0f * zoom_level / screen_size.y, 0.0f);
        glm::vec3 translation(-center.x, -center.y, 0.0f);

        auto matrix = glm::scale(glm::mat4(), scale);
        matrix = glm::rotate(matrix, -rotation, { 0.0f, 0.0f, 1.0f });
        matrix = glm::translate(matrix, translation);

        return matrix;
      }
    }

    void SceneRenderer::render(const Viewport& viewport, Vector2u screen_size, double frame_progress) const
    {
      if (!impl_->rendering_initialized_)
      {
        impl_->initialize_track_buffers();
        impl_->initialize_entity_buffers();
        impl_->initialize_particle_buffers();

        impl_->rendering_initialized_ = true;
      }

      // Set the shader parameters for the drawable entities
      // Apply the interpolation

      // For each view
      //   Apply the view
      //   Draw the track scene
      //   Draw the dynamic scene


      glDisable(GL_CULL_FACE);      
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glDisable(GL_DEPTH_TEST);
      glDisable(GL_STENCIL_TEST);
      glEnable(GL_SCISSOR_TEST);
      
      glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
      glClearDepth(0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      auto screen_rect = rect_cast<float>(viewport.screen_rect());

      Rect<GLint> scissor_region =
      {
        static_cast<GLint>(std::round(screen_size.x * screen_rect.left)),
        static_cast<GLint>(std::round(screen_size.y - (screen_size.y * screen_rect.bottom()))),
        static_cast<GLint>(std::round(screen_size.x * screen_rect.width)),
        static_cast<GLint>(std::round(screen_size.y * screen_rect.height))
      };

      glScissor(scissor_region.left, scissor_region.top, scissor_region.width, scissor_region.height);

      auto world_size = impl_->track_scene->track_size();
      auto view_mat = detail::compute_matrix(viewport, world_size, screen_size, frame_progress);

      const float shadow_color[4] = { 0.2f, 0.2f, 0.2f, 0.5f };

      glUseProgram(impl_->shadow_shader_program.get());
      glUniform4fv(impl_->shadow_color_location, 1, shadow_color);
      glUniformMatrix4fv(impl_->shadow_view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_mat));
      glUniform1f(impl_->shadow_frame_progress_location, static_cast<GLfloat>(frame_progress));

      glUseProgram(impl_->car_shader_program.get());
      glUniformMatrix4fv(impl_->car_view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_mat));
      glUniform1f(impl_->car_frame_progress_location, static_cast<GLfloat>(frame_progress));

      glUseProgram(impl_->particle_shader_program.get());
      glUniformMatrix4fv(impl_->particle_view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_mat));

      glUseProgram(impl_->track_shader_program.get());
      glUniformMatrix4fv(impl_->track_view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_mat));  

      const auto& components = impl_->track_scene->components();

      auto draw_entity_groups = [=](auto group_begin, auto group_end)
      {  
        glBindVertexArray(impl_->entity_vertex_array.get());
        glActiveTexture(GL_TEXTURE0);
        glBindSampler(0, impl_->texture_sampler.get());
        glActiveTexture(GL_TEXTURE1);
        glBindSampler(1, impl_->color_sampler.get());
        glBindTexture(GL_TEXTURE_2D, impl_->dynamic_scene->color_scheme_texture()->get());
        glActiveTexture(GL_TEXTURE0);

        for (; group_begin != group_end; ++group_begin)
        {
          auto group = *group_begin;
          glUseProgram(impl_->shadow_shader_program.get());

          glBindTexture(GL_TEXTURE_2D, group.texture->get());

          auto draw_offset = impl_->entity_buffer_draw_offset + sizeof(GLuint) * group.element_index;
          glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(group.element_count), GL_UNSIGNED_INT,
                         reinterpret_cast<const void*>(draw_offset));

          
          if (group.shader_type == detail::ShaderType::Car)
          {
            glUseProgram(impl_->car_shader_program.get());
          }

          glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(group.element_count), GL_UNSIGNED_INT,
                         reinterpret_cast<const void*>(draw_offset));
        }
      };

      auto& entity_groups = impl_->entity_groups;
      auto entity_group_it = entity_groups.begin();      

      std::size_t offset = 0;
      for (auto range_start = components.begin(); range_start != components.end(); )
      {
        std::size_t current_level = range_start->level;

        // Draw all the components that are on the current level
        glUseProgram(impl_->track_shader_program.get());

        glActiveTexture(GL_TEXTURE0);
        glBindSampler(0, impl_->texture_sampler.get());
        glBindVertexArray(impl_->track_vertex_array.get());
        for (; range_start != components.end() && current_level == range_start->level; ++range_start)
        {
          const auto& component = *range_start;
          glBindTexture(GL_TEXTURE_2D, component.texture->get());
          glDrawArrays(GL_TRIANGLES, static_cast<GLint>(offset),
                       static_cast<GLsizei>(component.vertex_count));
          
          offset += component.vertex_count;
        }

        if (range_start != components.end())
        {
          std::size_t next_level = range_start->level;

          // And find all the entity groups that are to be drawn at this point.
          // That is, all the entity groups with a level that's smaller than the next component's level.
          auto entity_group_end = std::find_if(entity_group_it, entity_groups.end(),
                                               [=](const auto& entity_group)
          {
            return entity_group.level >= next_level;
          });

          draw_entity_groups(entity_group_it, entity_group_end);

          entity_group_it = entity_group_end;
        }       
      }

      draw_entity_groups(entity_group_it, entity_groups.end());

      glBindSampler(0, 0);
      glBindSampler(1, 0);
      glBindTexture(GL_TEXTURE_2D, 0);

      if (impl_->particle_index_count != 0)
      {
        glUseProgram(impl_->particle_shader_program.get());
        glBindVertexArray(impl_->particle_vertex_array.get());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, impl_->particle_vertex_buffer.get());

        auto buffer_offset = impl_->particle_buffer_draw_offset;

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(impl_->particle_index_count), GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(buffer_offset));
      }

      glUseProgram(0);
      glBindVertexArray(0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void SceneRenderer::update(std::uint32_t frame_duration)
    {
      double fd = frame_duration * 0.001;

      // What we need here:
      // Query the DynamicScene for the entities that need to be drawn
      // Populate our vertex buffer object with the needed data

      auto& drawable_cache = impl_->drawable_cache;
      drawable_cache.clear();
      for (std::size_t instance_id = 0, count = impl_->dynamic_scene->entity_count(); instance_id != count; ++instance_id)
      {
        drawable_cache.push_back(impl_->dynamic_scene->entity_info(instance_id));
      }

      if (!drawable_cache.empty())
      {
        std::sort(drawable_cache.begin(), drawable_cache.end(),
                  [](const DrawableEntity& a, const DrawableEntity& b)
        {
          auto type_a = detail::entity_type_to_shader_type(a.entity->type());
          auto type_b = detail::entity_type_to_shader_type(b.entity->type());

          auto level_a = a.entity->z_level();
          auto level_b = b.entity->z_level();

          auto tuple_a = std::tie(level_a, type_a);
          auto tuple_b = std::tie(level_b, type_b);

          std::less<> less;
          if (tuple_a == tuple_b) return less(a.texture, b.texture);

          return tuple_a < tuple_b;
        });

        auto& entity_groups = impl_->entity_groups;
        entity_groups.clear();

        auto entity_vertex_count = drawable_cache.size() * detail::vertices_per_entity;
        auto entity_index_count = drawable_cache.size() * detail::indices_per_entity;        

        auto range_size = entity_vertex_count * sizeof(EntityVertex) + sizeof(GLuint) * entity_index_count;
        if (auto mod = range_size % sizeof(EntityVertex))
        {
          range_size -= mod;
          range_size += sizeof(EntityVertex);
        }

        auto buffer_offset = impl_->entity_buffer_offset;

        glBindBuffer(GL_ARRAY_BUFFER, impl_->entity_vertex_buffer.get());
        if (buffer_offset + range_size > detail::entity_buffer_size)
        {
          impl_->entity_buffer_offset = 0;
          impl_->entity_vertex_index = 0;
          buffer_offset = 0;
          glBufferData(GL_ARRAY_BUFFER, detail::entity_buffer_size, nullptr, GL_DYNAMIC_DRAW);
        }

        auto buffer_flags = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
        
        auto mapping = glMapBufferRange(GL_ARRAY_BUFFER, buffer_offset, range_size, buffer_flags);
        auto entity_vertex_ptr = static_cast<EntityVertex*>(mapping);
        
        auto entity_index_ptr = reinterpret_cast<GLuint*>(static_cast<char*>(mapping) + 
                                                          sizeof(EntityVertex) * entity_vertex_count);        

        if (mapping)
        {
          GLuint element_index = 0;
          GLuint vertex_index = static_cast<GLuint>(impl_->entity_buffer_offset / sizeof(EntityVertex));

          // Now, group the drawable entities by shader/texture combination, so that we can render them efficiently.
          auto group_start = drawable_cache.begin();
          while (group_start != drawable_cache.end())
          {
            auto find_group_end = [](const DrawableEntity& drawable, detail::ShaderType type,
                                     const graphics::Texture* texture, std::uint32_t level)
            {
              return level != drawable.entity->z_level() || texture != drawable.texture ||
                type != detail::entity_type_to_shader_type(drawable.entity->type());
            };

            auto shader_type = detail::entity_type_to_shader_type(group_start->entity->type());
            auto texture = group_start->texture;
            auto level = group_start->entity->z_level();

            auto group_end = std::find_if(std::next(group_start), drawable_cache.end(),
                                          std::bind(find_group_end, std::placeholders::_1, shader_type, texture, level));

            entity_groups.emplace_back();

            auto entity_count = static_cast<GLuint>(std::distance(group_start, group_end));

            auto& group = entity_groups.back();
            group.shader_type = shader_type;
            group.texture = texture;
            group.level = level;
            group.element_index = element_index;            
            group.element_count = entity_count * detail::indices_per_entity;

            detail::generate_entity_indices(entity_index_ptr + element_index, entity_count, vertex_index);

            // Generate all the vertices for this "group"
            for (; group_start != group_end; ++group_start)
            {
              detail::generate_entity_vertices(*group_start, entity_vertex_ptr, group_start->frame_offset);

              entity_vertex_ptr += detail::vertices_per_entity;
            }            
          }

          glUnmapBuffer(GL_ARRAY_BUFFER);
          glBindBuffer(GL_ARRAY_BUFFER, 0);

          impl_->entity_vertex_index = vertex_index;
          impl_->entity_buffer_offset = buffer_offset + range_size;
          impl_->entity_buffer_draw_offset = buffer_offset + sizeof(EntityVertex) * entity_vertex_count;
          impl_->entity_buffer_range_size = range_size;
        }

        else
        {
          impl_->entity_vertex_index = 0;
          impl_->entity_buffer_offset = 0;
          impl_->entity_buffer_draw_offset = 0;
          impl_->entity_buffer_range_size = 0;
        }
      }
    }

    namespace detail
    {
      static void generate_entity_indices(GLuint* index_ptr, GLuint entity_count, GLuint vertex_index)
      {
        while (entity_count-- != 0)
        {
          index_ptr[0] = vertex_index;
          index_ptr[1] = vertex_index + 1;
          index_ptr[2] = vertex_index + 2;
          index_ptr[3] = index_ptr[2];
          index_ptr[4] = index_ptr[0];
          index_ptr[5] = vertex_index + 3;

          index_ptr += detail::indices_per_entity;
          vertex_index += detail::vertices_per_entity;
        }
      }

      static void generate_entity_vertices(const DrawableEntity& drawable_entity, EntityVertex* vertices,
                                           Vector2<float> frame_offset)
      {
        const auto& texture_rect = drawable_entity.texture_coords;
        const auto& colorizer_rect = drawable_entity.colorizer_coords;
        const auto& frame_bounds = drawable_entity.frame_bounds;

        Vector2<float> half_size(texture_rect.width * 0.5f, texture_rect.height * 0.5f);
        {
          auto top_left = drawable_entity.transformation * glm::vec4(-half_size.x, -half_size.y, 1.0f, 1.0f);
          auto bottom_right = drawable_entity.transformation * glm::vec4(half_size.x, half_size.y, 1.0f, 1.0f);
          auto bottom_left = drawable_entity.transformation * glm::vec4(-half_size.x, half_size.y, 1.0f, 1.0f);
          auto top_right = drawable_entity.transformation * glm::vec4(half_size.x, -half_size.y, 1.0f, 1.0f);

          vertices[0].position = { top_left.x, top_left.y };
          vertices[1].position = { bottom_left.x, bottom_left.y };
          vertices[2].position = { bottom_right.x, bottom_right.y };
          vertices[3].position = { top_right.x, top_right.y };
        }

        vertices[0].texture_coords = { texture_rect.left, texture_rect.top };
        vertices[1].texture_coords = { texture_rect.left, texture_rect.bottom() };
        vertices[2].texture_coords = { texture_rect.right(), texture_rect.bottom() };
        vertices[3].texture_coords = { texture_rect.right(), texture_rect.top };

        {
          const auto& transform = drawable_entity.colorizer_transformation;

          glm::vec2 center(0.5f, 0.5f);

          auto left = -center.x;
          auto top = -center.y;
          auto right = center.x;
          auto bottom = center.y;

          glm::vec2 topleft_bounds(frame_bounds.left, frame_bounds.top);
          glm::vec2 bounds_size(1.0f / frame_bounds.width, 1.0f / frame_bounds.height);

          auto top_left = ((transform * glm::vec2(left, top) + center) - topleft_bounds) * bounds_size;
          auto bottom_left = ((transform * glm::vec2(left, bottom) + center) - topleft_bounds) * bounds_size;
          auto bottom_right = ((transform * glm::vec2(right, bottom) + center)- topleft_bounds) * bounds_size;
          auto top_right = ((transform * glm::vec2(right, top) + center) - topleft_bounds) * bounds_size;

          // Need to transform colorizer coords according to frame bounds
          vertices[0].colorizer_coords = { top_left.x, top_left.y };
          vertices[1].colorizer_coords = { bottom_left.x, bottom_left.y };
          vertices[2].colorizer_coords = { bottom_right.x, bottom_right.y };
          vertices[3].colorizer_coords = { top_right.x, top_right.y };
        }

        FloatRect colorizer_bounds =
        {
          colorizer_rect.left + frame_bounds.left * colorizer_rect.width,
          colorizer_rect.top + frame_bounds.top * colorizer_rect.height,
          colorizer_rect.width * frame_bounds.width,
          colorizer_rect.height * frame_bounds.height
        };

        for (std::size_t n = 0; n != 4; ++n)
        {
          auto& vertex = vertices[n];

          vertex.frame_offset = frame_offset;
          vertex.colorizer_bounds = colorizer_bounds;

          const auto& colors = drawable_entity.colors;
          vertex.primary_color = colors[0];
          vertex.secondary_color = colors[1];
          vertex.tertiary_color = colors[2];
        }
      }

      static ShaderType entity_type_to_shader_type(world::EntityType entity_type)
      {
        using world::EntityType;
        switch (entity_type)
        {
        case EntityType::Car:
          return ShaderType::Car;

        default:
          return ShaderType::None;
        }
      }
    }
  }
}