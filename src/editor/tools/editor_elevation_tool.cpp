/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "editor_elevation_tool.hpp"

#include "editor/height_map_shaders.hpp"
#include "editor/terrain_interface_shaders.hpp"
#include "editor/editor_scene.hpp"

#include "editor/track_3d.hpp"
#include "editor/height_map.hpp"

#include "graphics/gl_scissor_box.hpp"

#include "user_interface/gui_input_state.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace ts
{
  namespace editor
  {
    namespace tools
    {
      namespace detail
      {
        template <typename Func>
        void iterate_height_map_tool_points(const resources_3d::HeightMap& height_map, 
                                            Vector2i center_position,
                                            const ElevationToolProperties& tool_properties,
                                            Func&& func)
        {
          auto cell_size = static_cast<std::int32_t>(height_map.cell_size());
          auto half_size = static_cast<std::int32_t>(tool_properties.size / 2);
          auto start = (center_position - half_size) / cell_size;
          auto end = (center_position + half_size + cell_size) / cell_size;
          for (auto y = start.y; y <= end.y; ++y)
          {
            for (auto x = start.x; x <= end.x; ++x)
            {
              auto map_coords = Vector2i(x, y);
              auto pos = map_coords * cell_size;
              auto offset = pos - center_position;
              auto distance = offset.x * offset.x + offset.y * offset.y;
              float strength = 1.0f;
              if (tool_properties.shape == ElevationToolShape::Circular)
              {
                if (distance > half_size * half_size)
                {                  
                  // If the point lies outside the circle's radius, the strength is 0.0.
                  strength = 0.0f;
                }

                else if (tool_properties.softness > 0.01f)
                {
                  // Otherwise, the strength depends on the distance and the softness.
                  // At the circle outline, the strength is 0. The softness determines
                  // how much of the circle edge should be interpolated.
                  auto ratio = distance / static_cast<float>(half_size * half_size);
                  auto softness = tool_properties.softness;

                  ratio = std::sqrt(1.0f - ratio);
                  if (ratio < softness)
                  {
                    strength = ratio / softness;
                  }
                }
              }

              else if (tool_properties.shape == ElevationToolShape::Square)
              {
                auto absolute_offset = make_vector2(std::abs(offset.x), std::abs(offset.y));
                if (absolute_offset.x > half_size || absolute_offset.y > half_size)
                {
                  strength = 0.0f;
                }

                else if (tool_properties.softness > 0.01f)
                {
                  auto ratio = std::max(absolute_offset.x, absolute_offset.y) / static_cast<float>(half_size);
                  auto softness = tool_properties.softness;

                  ratio = 1.0f - ratio;
                  if (ratio < softness)
                  {
                    strength = ratio / softness;
                  }
                }
              }
              
              func(map_coords, pos, strength);
            }
          }
        }

        boost::optional<IntRect> apply_heightening(EditorScene& editor_scene, Vector2f position,
                                                   const ElevationToolProperties& tool_properties)
        {
          auto& track = editor_scene.track();
          const auto& height_map = track.height_map();

          auto max_z = static_cast<float>(editor_scene.track().size().z);
          auto map_size = track.height_map().size();

          boost::optional<IntRect> modified_area;
          auto point_func = [&](Vector2i coord, Vector2i position, float strength)
          {
            auto rect = IntRect(coord, Vector2i(1, 1));
            if (!modified_area)
            {
              modified_area.emplace(rect);
            }

            else
            {
              *modified_area = combine(*modified_area, rect);
            }

            track.raise_elevation_at(vector2_cast<std::uint32_t>(coord),
                                     strength * tool_properties.strength);
          };

          // Make the terrain around the specified position higher.
          iterate_height_map_tool_points(height_map, vector2_cast<std::int32_t>(position),
                                         tool_properties, point_func);

          return modified_area;
        }

        boost::optional<IntRect> apply_lowering(EditorScene& editor_scene, Vector2f position,
                                                const ElevationToolProperties& tool_properties)
        {
          auto inverted_properties = tool_properties;
          inverted_properties.strength = -inverted_properties.strength;
          return apply_heightening(editor_scene, position, inverted_properties);
        }

        boost::optional<IntRect> apply_equalization(EditorScene& editor_scene, Vector2f position,
                                                    const ElevationToolProperties& tool_properties)
        {
          return boost::none;
        }


        static auto create_elevation_shader_program()
        {
          const auto version_string = "#version 330\n";

          const char* const vertex_shaders[] =
          {
            version_string,
            scene_3d::height_map_vertex_shader_functions,
            terrain_interface_vertex_shader
          };

          const char* const fragment_shaders[] =
          {
            version_string,
            terrain_interface_fragment_shader
          };

          return graphics::create_shader_program(vertex_shaders, fragment_shaders);
        }
      }

      ElevationTool::ElevationTool(EditorScene* editor_scene)
        : EditorTool(editor_scene),
          interface_shader_(detail::create_elevation_shader_program()),
          interface_vertex_buffer_(graphics::create_buffer()),
          interface_index_buffer_(graphics::create_buffer()),
          height_map_sampler_(scene_3d::create_height_map_sampler())
      {}

      bool ElevationTool::update_gui(bool has_focus, const gui::InputState& input_state,
                                     gui::Geometry& geometry)
      {        
        // When the track is being clicked (and held), modify the height map, depending on the current mode. 
        // Every mode has its own adjustment function, which changes the elevation at a 
        // specific point of the map. Then, we have to commit the changes in order to
        // update the height map texture in the render scene.
        if (contains(editor_scene()->view_port(), input_state.mouse_position))
        {
          bool update = false;

          auto world_pos = editor_scene()->terrain_position_at(input_state.mouse_position);
          if (has_focus && world_pos)
          {
            Vector2f world_pos_2d(world_pos->x, world_pos->y);

            if (click_state(input_state, gui::MouseButton::Left))
            {
              auto func = [=]() -> decltype(&detail::apply_heightening)
              {
                switch (active_mode_)
                {
                case Heighten: return detail::apply_heightening;
                case Lower: return detail::apply_lowering;
                case Equalize: return detail::apply_equalization;
                default: return nullptr;
                }
              }();

              if (func)
              {
                if (auto modified_area = apply_height_map_function(world_pos_2d, func))
                {
                  if (!modified_area_) modified_area_ = modified_area;
                  else *modified_area_ = combine(*modified_area_, *modified_area);

                  update = true;
                }
              }
            }

            if (input_state.mouse_position != input_state.old_mouse_position)
            {
              update = true;
            }

            if (update)
            {
              update_interface_buffer(world_pos_2d);

              if (modified_area_)
              {
                editor_scene()->rebuild_height_map(*modified_area_);
                modified_area_ = boost::none;
              }
            }
          }
        }

        return has_focus;
      }

      void ElevationTool::set_active_mode(std::size_t mode)
      {
        active_mode_ = static_cast<Mode>(mode);
      }

      template <typename Function>
      boost::optional<IntRect> ElevationTool::apply_height_map_function(Vector2f world_pos, Function function)
      {
        return function(*editor_scene(), world_pos, tool_properties_);
      }

      void ElevationTool::update_interface_buffer(Vector2f hover_position)
      {
        map_coord_interface_info_.clear();
        vertex_cache_.clear();
        index_cache_.clear();

        auto add_coord_info = [&](Vector2i map_coords, Vector2i position, float strength)
        {
          MapCoordInterfaceInfo info;
          info.coords = map_coords;
          info.position = position;
          info.strength = strength;
          map_coord_interface_info_.push_back(info);
        };

        auto make_vertex = [](auto position, float strength)
        {          
          InterfaceVertex vertex;
          vertex.position = vector2_cast<float>(position);
          vertex.color = { 100, 255, 255, static_cast<std::uint8_t>(strength * 255) };
          return vertex;
        };

        const auto& height_map = editor_scene()->track().height_map();
        detail::iterate_height_map_tool_points(height_map, vector2_cast<std::int32_t>(hover_position),
                                               tool_properties_, add_coord_info);

        // We have the coordinates and their strengths, sort them by their x coordinate
        // so that we can generate the vertical lines
        std::sort(map_coord_interface_info_.begin(), map_coord_interface_info_.end(),
                  [](const auto& a, const auto& b)
        {
          return std::tie(a.coords.x, a.coords.y) < std::tie(b.coords.x, b.coords.y);
        });

        for (auto it = map_coord_interface_info_.begin(), end = map_coord_interface_info_.end(); it != end;)
        {
          auto x = it->coords.x;
          auto vertex_index = static_cast<std::uint32_t>(vertex_cache_.size());

          auto next_it = std::next(it);          
          for (; next_it != end && next_it->coords.x == x; ++it, ++next_it)
          {            
            vertex_cache_.push_back(make_vertex(it->position, it->strength));
            index_cache_.push_back(vertex_index);
            index_cache_.push_back(vertex_index + 1);
            ++vertex_index;            
          }
          
          vertex_cache_.push_back(make_vertex(it->position, it->strength));
          ++it;
        }

        // Now do the same for the horizontal lines.
        std::sort(map_coord_interface_info_.begin(), map_coord_interface_info_.end(),
                  [](const auto& a, const auto& b)
        {
          return std::tie(a.coords.y, a.coords.x) < std::tie(b.coords.y, b.coords.x);
        });

        for (auto it = map_coord_interface_info_.begin(), end = map_coord_interface_info_.end(); it != end;)
        {
          auto y = it->coords.y;
          auto vertex_index = static_cast<std::uint32_t>(vertex_cache_.size());

          auto next_it = std::next(it);
          for (; next_it != end && next_it->coords.y == y; ++it, ++next_it)
          {
            vertex_cache_.push_back(make_vertex(it->position, it->strength));
            index_cache_.push_back(vertex_index);
            index_cache_.push_back(vertex_index + 1);
            ++vertex_index;
          }

          vertex_cache_.push_back(make_vertex(it->position, it->strength));
          ++it;
        }

        interface_element_count_ = static_cast<std::uint32_t>(index_cache_.size());

        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, interface_index_buffer_.get()));
        glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_cache_.size() * sizeof(GLuint),
                             index_cache_.data(), GL_STATIC_DRAW));

        glCheck(glBindBuffer(GL_ARRAY_BUFFER, interface_vertex_buffer_.get()));
        glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_cache_.size() * sizeof(InterfaceVertex),
                             vertex_cache_.data(), GL_STATIC_DRAW));
      }

      void ElevationTool::render() const
      {
        glCheck(glDisable(GL_DEPTH_TEST));

        graphics::scissor_box(editor_scene()->screen_size(), editor_scene()->view_port());

        glCheck(glUseProgram(interface_shader_.get()));
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, interface_vertex_buffer_.get()));
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, interface_index_buffer_.get()));

        glCheck(glEnableVertexAttribArray(0));
        glCheck(glEnableVertexAttribArray(1));

        glCheck(glBindSampler(1, height_map_sampler_.get()));

        const auto& render_scene = editor_scene()->render_scene();
        const auto& terrain_scene = render_scene.terrain_scene();

        auto projection_matrix = render_scene.projection_matrix();
        auto view_matrix = render_scene.view_matrix();

        glCheck(glUniformMatrix4fv(glGetUniformLocation(interface_shader_.get(), "u_projectionMatrix"),
                                   1, GL_FALSE, glm::value_ptr(projection_matrix)));

        glCheck(glUniformMatrix4fv(glGetUniformLocation(interface_shader_.get(), "u_viewMatrix"),
                                   1, GL_FALSE, glm::value_ptr(view_matrix)));

        glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(InterfaceVertex),
                                      reinterpret_cast<const void*>(offsetof(InterfaceVertex, position))));

        glCheck(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(InterfaceVertex),
                                      reinterpret_cast<const void*>(offsetof(InterfaceVertex, color))));

        glCheck(glDrawElements(GL_LINES, interface_element_count_, GL_UNSIGNED_INT,
                               reinterpret_cast<const void*>(0)));

        glCheck(glDisableVertexAttribArray(0));
        glCheck(glDisableVertexAttribArray(1));

        graphics::disable_scissor_box();
      }
    }
  }
}