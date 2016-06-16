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

#include "user_interface/gui_background_style.hpp"
#include "user_interface/gui_style.hpp"
#include "user_interface/gui_widgets.hpp"
#include "user_interface/gui_geometry.hpp"

#include "utility/vector2.hpp"
#include "utility/vector3.hpp"
#include "utility/color.hpp"
#include "utility/random.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <boost/container/small_vector.hpp>

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

      static auto create_path_shader_program()
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

      void PathTool::set_active_mode(std::size_t mode)
      {
        auto new_mode = static_cast<Mode>(mode);
        if (new_mode != active_mode_)
        {
          active_mode_ = static_cast<Mode>(mode);

          node_transformation_ = boost::none;
          node_placement_ = boost::none;

          update_path_buffer();
        }
      }

      std::size_t PathTool::active_mode() const
      {
        return static_cast<std::size_t>(active_mode_);
      }

      void PathTool::update_gui(bool has_focus, const gui::InputState& input_state,
                                gui::Geometry& geometry)
      {
        // Depending on the current mode, render different GUI components.
        switch (active_mode_)
        {
        case Mode::Nodes:
          update_gui_path_nodes(has_focus, input_state, geometry);
          break;

        case Mode::StrokeSegments:
          update_gui_stroke_segments(has_focus, input_state, geometry);
          break;
        }
      }

      void PathTool::update_gui_path_nodes(bool& has_focus, const gui::InputState& input_state,
                                           gui::Geometry& geometry)
      {
        using namespace gui;
        auto scene = editor_scene();
        auto view_port = scene->view_port();

        auto world_pos = scene->screen_to_terrain_position(input_state.mouse_position);

        auto selected_path = scene->selected_track_path();
        if (selected_path == nullptr) return;
        
        // If the selected path has suddenly become different, we need to drop all of our transformations
        if (selected_path != selected_path_)
        {
          if (node_transformation_)
          {
            apply_node_transformation(*selected_path_, *node_transformation_);
            node_transformation_ = boost::none;
          }

          node_placement_ = boost::none;
          selected_path_ = selected_path;
        }

        auto& path = *selected_path;
        if (node_transformation_ && world_pos)
        {
          // If there's a node being transformed and we moved the mouse...
          if (input_state.mouse_position != input_state.old_mouse_position)
          {
            node_transformation_->end_point = { world_pos->x, world_pos->y };
            update_node_transformation(path, *node_transformation_);
          }

          // If the mouse isn't being clicked anymore, finish the transformation.
          if (!click_state(input_state, gui::MouseButton::Left))
          {
            apply_node_transformation(path, *node_transformation_);
            node_transformation_ = boost::none;
          }
        }

        if (node_placement_ && world_pos)
        {
          // If there's a node being placed and we moved the mouse...
          if (input_state.mouse_position != input_state.old_mouse_position)
          {
            node_placement_->end_point = { world_pos->x, world_pos->y };
            update_node_placement(path, *node_placement_);
          }

          if (!click_state(input_state, gui::MouseButton::Left))
          {
            apply_node_placement(path, *node_placement_);
            node_placement_ = boost::none;
          }
        }

        auto node_style = styles::fill_area(Colorb(100, 255, 255, 120)) +
          make_hover_style(styles::fill_area(Colorb(100, 255, 255, 220)));

        // For each node, draw an interactive square at the corresponding screen position.
        if (auto selected_path = scene->selected_track_path())
        {
          const auto& path = *selected_path;

          std::size_t node_index = 0;
          for (const auto& node : path.nodes)
          {
            auto process_control = [&](Vector2f position,
                                       NodeTransformation::Control control,
                                       std::int32_t widget_size)
            {
              auto screen_position = scene->world_to_screen_position(position);

              // If the position is contained within the view port, display a little widget
              if (contains(view_port, screen_position))
              {
                auto rect = make_rect_from_points(screen_position - make_vector2(widget_size, widget_size),
                                                  screen_position + make_vector2(widget_size, widget_size));

                auto state = widgets::button(rect_cast<float>(rect), node_style, input_state, geometry);
                if (has_focus && click_state(state) && !node_transformation_ && !node_placement_)
                {
                  // If this widget is being clicked, set it as the selected node, and also
                  // mark this node as the one being dragged.
                  node_transformation_.emplace();
                  node_transformation_->control = control;
                  node_transformation_->node_index = node_index;
                  node_transformation_->control_point = position;
                  node_transformation_->start_point = { world_pos->x, world_pos->y };
                  node_transformation_->end_point = { world_pos->x, world_pos->y };
                }
              }
            };

            process_control(node.position, NodeTransformation::Node, 6);
            process_control(node.first_control, NodeTransformation::FirstControl, 4);
            process_control(node.second_control, NodeTransformation::SecondControl, 4);

            ++node_index;
          }
        }

        // Now, if we didn't click on any nodes (that is, we don't have an active node transformation),
        // and we still have focus here, see if the viewport area was clicked,
        // so that we can initiate the placement of a new node.
        if (has_focus && !node_transformation_ && !node_placement_)
        {
          auto state = gui::widget_state(rect_cast<float>(view_port), input_state);
          if (click_state(state))
          {
            if (auto world_pos = scene->screen_to_terrain_position(input_state.mouse_position))
            {
              node_placement_.emplace();
              node_placement_->end_point = { world_pos->x, world_pos->y };

              path.nodes.emplace_back();
              auto& node = path.nodes.back();

              node.position = node_placement_->end_point;
              node.first_control = node.position;
              node.second_control = node.position;
              node.width = utility::random_real(path.min_width, path.max_width);  
            }
          }
        }
      }

      void PathTool::update_gui_stroke_segments(bool& has_focus, const gui::InputState& input_state,
                                                gui::Geometry& geometry)
      {
        using namespace gui;

        auto placement_node_style = styles::fill_area(Colorb(100, 255, 255, 220));

        auto transform_node_style = styles::fill_area(Colorb(255, 255, 255, 120)) +
          make_hover_style(styles::fill_area(Colorb(255, 255, 255, 220)));

        // For every segment of the currently selected stroke style,
        // display a start and end handle and make these draggable.
        auto scene = editor_scene();
        auto stroke_index = scene->selected_track_path_stroke_index();
        auto selected_path = scene->selected_track_path();
        if (!selected_path || stroke_index >= selected_path->strokes.size()) return;

        auto& path = *selected_path;
        auto& stroke = path.strokes[stroke_index];
        auto& stroke_properties = stroke.properties;

        auto world_pos = scene->screen_to_terrain_position(input_state.mouse_position);
        if (stroke_properties.is_segmented && world_pos)
        {
          for (const auto& segment : stroke.segments)
          {
            // Draw a handle widget at the start and end positions for every segment
          }

          if (!segment_transformation_)
          {
            // If the mouse is currently somewhere over the path, display an interactive handle that
            // initiates the placement of a segment when clicked.

            // Find a matching path point where distance to the mouse position is
            // smaller than a specified tolerance value.
            auto world_pos = scene->screen_to_terrain_position(input_state.mouse_position);
            if (world_pos)
            {
              auto pos_2d = make_vector2(world_pos->x, world_pos->y);
              const auto widget_size = Vector2i(5, 5);

              // Need to search the path in three places. Two on the outside of the path, and one
              // on the path itself.
              boost::container::small_vector<std::pair<float, SegmentSide>, 4> sides =
              {
                { 0.0f, SegmentSide::Both },
                { -1.0f, SegmentSide::A },
                { 1.0f, SegmentSide::B }
              };

              // Unless of course there's a segment already being placed, in which case 
              // we only have to search that side.
              if (segment_placement_)
              {
                // Simply remove the entires that don't match up.
                sides.erase(std::remove_if(sides.begin(), sides.end(), [=](const auto& side)
                {
                  return side.second != segment_placement_->side;
                }), sides.end());
              }

              for (auto side : sides)
              {
                using namespace resources_3d;
                auto match = find_first_matching_path_position(path.nodes.begin(), path.nodes.end(),
                                                               pos_2d, side.first, 5.0f);
                if (match.node_it != path.nodes.end())
                {
                  auto widget_pos = scene->world_to_screen_position(match.point);
                  auto widget_rect = make_rect_from_points(widget_pos - widget_size,
                                                           widget_pos + widget_size);
                  auto state = widgets::button(rect_cast<float>(widget_rect), placement_node_style,
                                               input_state, geometry);
                  if (was_clicked(input_state))
                  {
                    auto node_index = static_cast<std::uint32_t>(std::distance(path.nodes.begin(), 
                                                                               match.node_it));
                    if (!segment_placement_)
                    {
                      // Initiate segment placement if the little handle was clicked
                      segment_placement_.emplace();
                      segment_placement_->start_point = match.point;
                      segment_placement_->side = side.second;
                      segment_placement_->start_time = match.time_point;
                      segment_placement_->start_index = node_index;
                    }

                    else
                    {
                      // Finalize the placement of a segment that's already being placed.
                      apply_segment_stroke(path, stroke, *segment_placement_,
                                           node_index, match.time_point);

                      segment_placement_ = boost::none;
                    }
                  }
                }
              }
            }

            if (segment_placement_)
            {
              // If there's a segment currently being placed, display a little reference
              auto style = styles::fill_area(Colorb(255, 150, 0, 220));
              auto widget_size = Vector2i(5, 5);

              auto point = scene->world_to_screen_position(segment_placement_->start_point);
              auto area = make_rect_from_points(point + widget_size,
                                                point - widget_size);

              add_background(rect_cast<float>(area), style, geometry);
            }
          }
        }
      }

      void PathTool::update_node_transformation(resources_3d::TrackPath& path,
                                                const NodeTransformation& transformation)
      {
        auto& node = path.nodes[transformation.node_index];
        using Control = NodeTransformation::Control;

        auto offset = transformation.end_point - transformation.start_point;
        switch (transformation.control)
        {
        case Control::FirstControl:
        {
          // Make sure the angle between to the node point is the same for both control points.
          node.first_control = transformation.control_point + offset;
          auto normal = normalize(node.position - node.first_control);
          node.second_control = node.position + distance(node.position, node.second_control) * normal;
          break;
        }

        case Control::SecondControl:
        {
          // Ditto here.
          node.second_control = transformation.control_point + offset;
          auto normal = normalize(node.position - node.second_control);
          node.first_control = node.position + distance(node.position, node.first_control) * normal;
          break;
        }

        case Control::Node:
        default:
        {
          // Update the position with the new end point and move the control points over
          // by the same offset, to ensure they keep their respective angles.
          auto old_position = node.position;
          node.position = transformation.control_point + offset;
          node.first_control += node.position - old_position;
          node.second_control += node.position - old_position;
          break;
        }
        }

        update_path_buffer();
      }

      void PathTool::apply_node_transformation(resources_3d::TrackPath& path,
                                                const NodeTransformation& transformation)
      {
        update_node_transformation(path, transformation);

        editor_scene()->commit(&path);
      }

      namespace detail
      {
        static void update_placement_node(resources_3d::TrackPathNode& node,
                                          Vector2f end_point)
        {
          node.second_control = end_point;
          node.first_control = node.position - (node.second_control - node.position);
        }
      }

      void PathTool::update_node_placement(resources_3d::TrackPath& path, 
                                           const NodePlacement& node_placement)
      {
        detail::update_placement_node(path.nodes.back(), node_placement.end_point);

        update_path_buffer();
      }

      void PathTool::apply_node_placement(resources_3d::TrackPath& path,
                                          const NodePlacement& node_placement)
      {
        auto& node = path.nodes.back();
        detail::update_placement_node(node, node_placement.end_point);

        update_path_buffer();        
        editor_scene()->commit(&path);
      }

      void PathTool::apply_segment_stroke(resources_3d::TrackPath& path,
                                          resources_3d::SegmentedStroke& stroke,
                                          const SegmentPlacement& segment_placement,
                                          std::uint32_t end_index, float end_time)
      {
        using Stroke = resources_3d::SegmentedStroke;
        using Segment = resources_3d::StrokeSegment;

        Segment segment;
        segment.side = Segment::Both;
        if (segment_placement.side == SegmentSide::A) segment.side = Segment::First;
        else if (segment_placement.side == SegmentSide::B) segment.side = Segment::Second;

        segment.start_index = segment_placement.start_index;
        segment.start_time_point = segment_placement.start_time;
        segment.end_index = end_index;
        segment.end_time_point = end_time;
        stroke.segments.push_back(segment);

        update_path_buffer();
        editor_scene()->commit(&path);
      }

      void PathTool::process_event(const event_type& event)
      {        
        if (auto selected_path = editor_scene()->selected_track_path())
        {
          auto& path = *selected_path;

          if (event.type == sf::Event::KeyPressed && active_mode_ == Mode::Nodes)
          {
            switch (event.key.code)
            {
            case sf::Keyboard::BackSpace:
              if (!path.nodes.empty())
              {
                path.nodes.pop_back();
                path.closed = false;
                update_path_buffer();
                editor_scene()->commit(selected_path);
              }

              break;

            case sf::Keyboard::C:
            {
              if (!path.nodes.empty() && !path.closed)
              {
                auto node = path.nodes.front();
                path.nodes.push_back(node);
                path.closed = true;

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

        if (auto selected_path = editor_scene()->selected_track_path())
        {
          const auto& path = *selected_path;
          using node_iterator = decltype(path.nodes.begin());

          // TODO: make it so that not everything has to be recalculated every time,
          // particularly when adding or modifying a node at the end of the path.
          std::vector<scene_3d::PathVertexPoint<node_iterator>> vertex_points;
          scene_3d::compute_path_vertex_points(path.nodes.begin(), path.nodes.end(),
                                               0.025f, vertex_points);

          std::vector<PathVertex> vertices;
          std::vector<GLuint> indices;

          const Colorb color = { 0, 200, 255, 50 };
          auto vertex_func = [=](Vector3f position, Vector2f tex_coord, Vector3f normal)
          {
            Vector2f pos_2d = { position.x, position.y };
            return PathVertex{ pos_2d, color };
          };

          // Generate a track outline
          resources_3d::StrokeProperties stroke;
          stroke.type = stroke.Border;
          stroke.use_relative_size = false;
          stroke.offset = -1.5f;
          stroke.width = 3.0f;
          stroke.color = color;
          scene_3d::generate_path_vertices(vertex_points, stroke,
                                           vertex_func, std::back_inserter(vertices),
                                           0, std::back_inserter(indices));


          Colorb line_color = { 100, 255, 255, 150 };
          element_count_ = indices.size();
          auto line_vertex = [=](Vector2f position) { return PathVertex{ position, line_color }; };

          line_element_count_ = 0;
          if (active_mode_ == Mode::Nodes)
          {
            // When editing nodes, draw lines between all nodes and their control points.
            auto line_index = static_cast<GLuint>(vertices.size());
            for (const auto& node : path.nodes)
            {
              vertices.push_back(line_vertex(node.first_control));
              vertices.push_back(line_vertex(node.position));
              vertices.push_back(line_vertex(node.second_control));

              indices.insert(indices.end(), 
              { 
                line_index, 
                line_index + 1, 
                line_index + 1, 
                line_index + 2 
              });

              line_index += 3;
              line_element_count_ += 4;
            }
          }

          // Also draw a line for the path itself.
          if (vertex_points.size() >= 2) 
          {
            auto path_color = Colorb(255, 255, 255, 100);
            auto path_line_vertex = [=](Vector2f position) { return PathVertex{ position, path_color }; };

            auto line_index = static_cast<GLuint>(vertices.size());

            // For all vertex points, add a line vertex
            for (const auto& point : vertex_points)
            {
              vertices.push_back(path_line_vertex(point.point));
            }            

            // And for every point segment, add two line indices.
            auto point_it = vertex_points.begin();
            auto next_point_it = std::next(point_it);
            for (; next_point_it != vertex_points.end(); ++point_it, ++next_point_it)
            {
              indices.insert(indices.end(), { line_index, line_index + 1 });

              line_element_count_ += 2;
              ++line_index;
            }
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