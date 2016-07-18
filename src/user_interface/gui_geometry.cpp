/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "gui_geometry.hpp"
#include "gui_renderer.hpp"

#include <glm/gtx/transform.hpp>

namespace ts
{
  namespace gui
  {
    void draw(const Renderer& renderer, const Geometry& geometry, const RenderState& render_state)
    {
      auto screen_size = render_state.screen_size;
      auto translation = render_state.translation;
      auto view_matrix = glm::translate(glm::mat4(), glm::vec3(-1.0f, 1.0f, 0.0f));
      view_matrix = glm::scale(view_matrix, glm::vec3(2.0f / screen_size.x, -2.0f / screen_size.y, 1.0f));
      view_matrix = glm::translate(view_matrix, glm::vec3(translation.x, translation.y, 0.0f));

      glViewport(0, 0, static_cast<GLsizei>(screen_size.x), static_cast<GLsizei>(screen_size.y));

      glDisable(GL_DEPTH_TEST);

      renderer.geometry_renderer().draw(geometry.geometry, view_matrix);
      renderer.font_renderer().draw(geometry.text, view_matrix);
    }

    void add_vertices(FloatRect area, Colorb color, Geometry& geometry)
    {
      auto right = area.right();
      auto bottom = area.bottom();

      graphics::Geometry::vertex_type vertices[6]
      {
        { { area.left, area.top },{}, color },
        { { area.left, bottom },{}, color },
        { { right, bottom },{}, color },
        { { right, area.top },{}, color }
      };

      vertices[4] = vertices[0];
      vertices[5] = vertices[2];

      geometry.geometry.add_vertices(vertices, 6, nullptr);
    }


    void add_vertical_gradient(FloatRect area, Colorb start_color, Colorb end_color,
                               Geometry& geometry)
    {
      auto right = area.right();
      auto bottom = area.bottom();

      graphics::Geometry::vertex_type vertices[6]
      {
        { { area.left, area.top },{}, start_color },
        { { area.left, bottom },{}, end_color },
        { { right, bottom },{}, end_color },
        { { right, area.top },{}, start_color }
      };

      vertices[4] = vertices[0];
      vertices[5] = vertices[2];

      geometry.geometry.add_vertices(vertices, 6, nullptr);
    }


    void add_vertices(FloatRect area, Colorb color, const graphics::Texture* texture,
                      IntRect texture_rect, Geometry& geometry)
    {
    }
  }
}