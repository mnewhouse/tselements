/*

* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "editor_path_tool.hpp"
#include "editor/editor_scene.hpp"
#include "editor/render_scene_3d.hpp"
#include "editor/terrain_scene_3d.hpp"
#include "editor/height_map_shaders.hpp"
#include "editor/path_vertices_detail.hpp"

#include "graphics/gl_check.hpp"
#include "graphics/gl_scissor_box.hpp"
#include "graphics/texture.hpp"

#include "utility/vector2.hpp"
#include "utility/vector3.hpp"
#include "utility/color.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace ts
{
  namespace editor
  {
    namespace tools
    {
      static const char* const path_vertex_shader = R"(
uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec4 in_color;
out vec4 frag_color;
void main()
{
    float z = heightAt(in_position);
    gl_Position = u_projectionMatrix * u_viewMatrix * vec4(in_position, z, 1.0);
    frag_color = in_color;
}
)";

      static const char* const path_fragment_shader = R"(
in vec4 frag_color;
out vec4 out_fragColor;
void main()
{
  out_fragColor = frag_color;
}
)";

      struct PathVertex
      {
        Vector2f position;
        Colorb color;
      };

      auto create_path_shader_program()
      {
        const auto version_string = "#version 330\n";

        const char* const vertex_shaders[] =
        {
          version_string,
          scene_3d::height_map_vertex_shader_functions,
          path_vertex_shader
        };

        const char* const fragment_shaders[] =
        {
          version_string,
          path_fragment_shader
        };

        return graphics::create_shader_program(vertex_shaders, fragment_shaders);
      }

      PathTool::PathTool(EditorScene* editor_scene)
        : EditorTool(editor_scene)
      {
        path_shader_ = create_path_shader_program();
        height_map_sampler_ = scene_3d::create_height_map_sampler();
        path_vertex_buffer_ = graphics::create_buffer();
        path_index_buffer_ = graphics::create_buffer();
      }

      void PathTool::process_event(const event_type& event)
      {
        auto selected_path = editor_scene()->selected_track_path();
        // On click-and-drag, add a new path node.
        if (selected_path_ != selected_path)
        {
          click_position_ = boost::none;
          selected_path_ = selected_path;
        }

        if (selected_path)
        {
          auto& path = *selected_path;
          
          if (event.type == sf::Event::MouseButtonReleased &&
              event.mouseButton.button == sf::Mouse::Left && click_position_)
          {
            Vector2i release_position(event.mouseButton.x, event.mouseButton.y);
            if (release_position != *click_position_)
            {
              if (auto pos = editor_scene()->screen_to_terrain_position(release_position))
              {
                auto& node = path.nodes.back();
                node.second_control = { pos->x, pos->y };
                node.first_control = node.position - (node.second_control - node.position);

                update_path_buffer();
                editor_scene()->commit(selected_path);
              }
            }

            click_position_ = boost::none;
          }

          else if (event.type == sf::Event::MouseButtonPressed &&
                   event.mouseButton.button == sf::Mouse::Left)
          {
            click_position_ = boost::none;
            Vector2i click_position = { event.mouseButton.x, event.mouseButton.y };
            if (auto pos = editor_scene()->screen_to_terrain_position(click_position))
            {
              click_position_.reset(click_position);

              resources_3d::TrackPathNode node;
              node.position = { pos->x, pos->y };
              node.first_control = node.position;
              node.second_control = node.first_control;
              node.width = 56.0f;
              path.nodes.push_back(node);
            }
          }

          else if (event.type == sf::Event::MouseMoved && click_position_)
          {
            Vector2i mouse_position(event.mouseMove.x, event.mouseMove.y);
            if (auto pos = editor_scene()->screen_to_terrain_position(mouse_position))
            {
              auto& node = path.nodes.back();
              node.second_control = { pos->x, pos->y };
              node.first_control = node.position - (node.second_control - node.position);

              update_path_buffer();
            }
          }

          else if (event.type == sf::Event::KeyPressed)
          {
            switch (event.key.code)
            {
            case sf::Keyboard::BackSpace:
              if (!path.nodes.empty())
              {
                path.nodes.pop_back();
                click_position_ = boost::none;
                update_path_buffer();
                editor_scene()->commit(selected_path);
              }

              break;

            case sf::Keyboard::C:
            {
              if (!selected_path_->nodes.empty())
              {
                auto node = path.nodes.front();
                path.nodes.push_back(node);

                update_path_buffer();
                editor_scene()->commit(selected_path);
              }

              break;
            }

            case sf::Keyboard::D:
              selected_path_ = nullptr;
              editor_scene()->commit(selected_path);
              editor_scene()->select_track_path(nullptr);
              update_path_buffer();
              break;
            }
          }
        }
      }


      void PathTool::update_path_buffer()
      {
        element_count_ = 0;
        line_element_count_ = 0;

        if (selected_path_)
        {
          const auto& path = *selected_path_;
          using node_iterator = decltype(path.nodes.begin());
          std::vector<scene_3d::PathVertexPoint<node_iterator>> vertex_points;

          scene_3d::compute_path_vertex_points(path.nodes.begin(), path.nodes.end(),
                                               0.1f, vertex_points);

          std::vector<PathVertex> vertices;
          std::vector<GLuint> indices;

          const Colorb color = { 0, 200, 255, 50 };
          auto vertex_func = [=](Vector2f position)
          {
            return PathVertex{ position, color };
          };

          resources_3d::TrackPathStroke stroke;
          stroke.type = stroke.Border;
          stroke.use_relative_size = false;
          stroke.width = 3.0f;
          stroke.color = color;
          scene_3d::generate_path_vertices(vertex_points, stroke,
                                           vertex_func, std::back_inserter(vertices),
                                           0, std::back_inserter(indices));


          Colorb line_color = { 255, 255, 255, 100 };
          element_count_ = indices.size();
          auto line_vertex = [=](Vector2f position) { return PathVertex{ position, line_color }; };

          line_element_count_ = 0;

          auto line_index = static_cast<GLuint>(vertices.size());
          for (const auto& node : path.nodes)
          {
            vertices.push_back(line_vertex(node.first_control));
            vertices.push_back(line_vertex(node.position));
            vertices.push_back(line_vertex(node.second_control));

            const GLuint line_indices[] = 
            { 
              line_index, 
              line_index + 1, 
              line_index + 1,
              line_index + 2 
            };

            indices.insert(indices.end(), std::begin(line_indices), std::end(line_indices));

            line_index += 3;
            line_element_count_ += 4;
          }

          glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, path_index_buffer_.get()));
          glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                               indices.data(), GL_STATIC_DRAW));

          glCheck(glBindBuffer(GL_ARRAY_BUFFER, path_vertex_buffer_.get()));
          glCheck(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PathVertex), 
                               vertices.data(), GL_STATIC_DRAW));          
        }
      }

      void PathTool::render() const
      {
        graphics::scissor_box(editor_scene()->screen_size(), editor_scene()->view_port());

        glCheck(glUseProgram(path_shader_.get()));
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, path_vertex_buffer_.get()));
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, path_index_buffer_.get()));

        glCheck(glEnableVertexAttribArray(0));
        glCheck(glEnableVertexAttribArray(1));

        glCheck(glBindSampler(1, height_map_sampler_.get()));

        const auto& render_scene = editor_scene()->render_scene();
        const auto& terrain_scene = render_scene.terrain_scene();
        const auto& height_map = terrain_scene.height_map_texture();
        glCheck(glActiveTexture(GL_TEXTURE1));
        glCheck(glBindTexture(GL_TEXTURE_2D, height_map.get()));

        auto projection_matrix = render_scene.projection_matrix();
        auto view_matrix = render_scene.view_matrix();

        glCheck(glUniformMatrix4fv(glGetUniformLocation(path_shader_.get(), "u_projectionMatrix"), 
                                   1, GL_FALSE, glm::value_ptr(projection_matrix)));

        glCheck(glUniformMatrix4fv(glGetUniformLocation(path_shader_.get(), "u_viewMatrix"), 
                                   1, GL_FALSE, glm::value_ptr(view_matrix)));

        glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(PathVertex),
                                      reinterpret_cast<const void*>(offsetof(PathVertex, position))));

        glCheck(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PathVertex),
                                      reinterpret_cast<const void*>(offsetof(PathVertex, color))));

        auto cell_size = terrain_scene.height_map_cell_size();
        auto max_z = terrain_scene.height_map_max_z();

        auto uniform_locations = scene_3d::get_height_map_uniform_locations(path_shader_);
        glCheck(glUniform1i(uniform_locations.height_map_sampler, 1));
        glCheck(glUniform1f(uniform_locations.height_map_max_z, max_z));
        glCheck(glUniform2f(uniform_locations.height_map_cell_size, cell_size.x, cell_size.y));

        glCheck(glDrawElements(GL_TRIANGLES, element_count_, GL_UNSIGNED_INT,
                               reinterpret_cast<const void*>(0)));

        glCheck(glDrawElements(GL_LINES, line_element_count_, GL_UNSIGNED_INT,
                               reinterpret_cast<const void*>(element_count_ * sizeof(GLuint))));

        glCheck(glDisableVertexAttribArray(0));
        glCheck(glDisableVertexAttribArray(1));

        graphics::disable_scissor_box();
      }
    }
  }
}