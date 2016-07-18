/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "track_edit_state.hpp"
#include "height_map.hpp"

#include "graphics/render_window.hpp"

#include "user_interface/gui_context.hpp"

#include "menu/update_input_state.hpp"

#include "resources/resource_store.hpp"

#include <iostream>

namespace ts
{
  namespace editor
  {
    namespace track
    {
      namespace detail
      {
        auto load_track()
        {
          auto height_map = resources_3d::generate_height_map(128, 16, 32.0f, 1.0f);

          resources_3d::Track track;
          track.adopt_height_map(std::move(height_map));

          track.resize({ 2048, 2048, 64 });

          track.define_texture(0, "editor/grass.png", IntRect(0, 0, 512, 512));
          track.define_texture(1, "editor/tarmac.png", IntRect(0, 0, 512, 512));
          track.define_texture(2, "editor/track_edge.png", IntRect(0, 0, 512, 512));
          track.define_texture(27, "editor/kerb.png", IntRect(0, 0, 32, 32));
          track.set_base_texture(0);

          using SegmentedStroke = resources_3d::SegmentedStroke;
          using Stroke = resources_3d::StrokeProperties;

          auto path = track.create_path();
          Stroke tarmac;
          tarmac.texture_id = 1;

          Stroke track_edge;
          track_edge.type = Stroke::Border;
          track_edge.use_relative_size = false;
          track_edge.width = 2.0f;
          track_edge.offset = -2.0f;
          track_edge.texture_id = 2;

          Stroke tarmac_edge;
          tarmac_edge.type = Stroke::Border;
          tarmac_edge.use_relative_size = false;
          tarmac_edge.width = 1.0f;
          tarmac_edge.offset = -2.5f;
          tarmac_edge.texture_id = 1;
          tarmac_edge.color = { 150, 150, 150, 255 };

          Stroke kerb;
          kerb.type = Stroke::Border;
          kerb.texture_mode = Stroke::Directional;
          kerb.is_segmented = true;
          kerb.use_relative_size = false;
          kerb.width = 5.0f;
          kerb.offset = -5.0f;
          kerb.texture_id = 27;
          kerb.texture_scale = 0.5f;
          kerb.color = { 255, 0, 0, 255 };
          kerb.inner_normal = 0.3f;
          kerb.outer_normal = 0.3f;
          kerb.bevel_width = 2.5f;
          kerb.bevel_strength = 0.5f;

          path->strokes.push_back(kerb);
          //path->strokes.push_back(tarmac_edge);
          path->strokes.push_back(track_edge);
          path->strokes.push_back(tarmac);

          path->min_width = 56.0f;
          path->max_width = 56.0f;

          return track;
        }
      }

      EditorState::EditorState(const game_context& ctx)
        : GameState(ctx),
          InterfaceState(Tool::Path, 0),
          editor_scene_(detail::load_track()),
          track_editor_menu_(&ctx.resource_store->font_library()),
          path_tool_(&editor_scene_),
          terrain_tool_(&editor_scene_),
          elevation_tool_(&editor_scene_),
          active_tool_(&path_tool_)
      {
        editor_scene_.load_scene();

        active_tool_->set_active_mode(active_mode());
      }

      void EditorState::render(const render_context& ctx) const
      {
        editor_scene_.render();

        if (active_tool_) active_tool_->render();

        auto game_context = context();
        const auto& gui_renderer = game_context.gui_context->renderer();

        gui::RenderState render_state;
        render_state.screen_size = vector2_cast<float>(game_context.render_window->size());
        render_state.translation = { 0.0f, 0.0f };

        gui::draw(gui_renderer, gui_geometry_, render_state);
      }

      void EditorState::update(const update_context& ctx)
      {
        // Update GUI
        clear(gui_geometry_);

        auto screen_size = context().render_window->size();

        auto menu_update_state = track_editor_menu_.update(*this, screen_size, input_state_, gui_geometry_);
        auto menu_area = menu_update_state.bounding_box;

        IntRect view_port(0, menu_area.bottom(), screen_size.x, screen_size.y - menu_area.bottom());
        editor_scene_.set_view_port(screen_size, view_port);

        bool has_focus = !menu_update_state.focus_state;
        if (active_tool_)
        {
          has_focus = active_tool_->update_gui(has_focus, input_state_, gui_geometry_);
        }

        // If our focus wasn't stolen by some other component, forward the events over to
        // the active tool object (if there is one).
        if (has_focus && active_tool_)
        {
          for (auto& event : event_queue_)
          {
            active_tool_->process_event(event);
          }
        }
        
        if (has_focus)
        {
          // If the mouse is being dragged with the right mouse button down...
          if (click_state(input_state_, gui::MouseButton::Right) &&
              old_click_state(input_state_, gui::MouseButton::Right) &&
              input_state_.mouse_position != input_state_.old_mouse_position &&
              contains(editor_scene_.view_port(), input_state_.old_mouse_position))
          {
            auto old_pos = editor_scene_.terrain_position_at(input_state_.old_mouse_position);
            auto new_pos = editor_scene_.terrain_position_at(input_state_.mouse_position);
            if (old_pos && new_pos)
            {
              auto offset = *old_pos - *new_pos;
              editor_scene_.move_camera_2d({ offset.x, offset.y });
            }
          }

          if (input_state_.mouse_wheel_delta != 0 && 
              contains(editor_scene_.view_port(), input_state_.mouse_position))
          {
            // If they scrolled the mouse while the editor canvas has focus, zoom in, or out.
            // We need to take the camera height into account, and use multiply that height
            // by some fixed amount.
            auto camera_pos = editor_scene_.camera_position();
            //auto terrain_height = interpolate_height_at(editor_scene_.track().height_map(),
            //                                            Vector2f(camera_pos.x, camera_pos.y));

            auto camera_height = camera_pos.z;
            auto new_camera_height = camera_height;
            
            // Zoom in 
            auto num_steps = std::abs(input_state_.mouse_wheel_delta);
            auto zoom_multiplier = input_state_.mouse_wheel_delta < 0 ? 1.1f : 1.0f / 1.1f;
            for (auto step = 0; step != num_steps; ++step)
            {
              new_camera_height *= zoom_multiplier;
            }

            new_camera_height = std::min(new_camera_height, 300.f);
            new_camera_height = std::max(new_camera_height, 10.0f);

            editor_scene_.move_camera(Vector3f(0.0f, 0.0f, new_camera_height - camera_height));
          }
        }

        // And empty the event queue.
        event_queue_.clear();

        menu::update_old_input_state(input_state_);
      }

      void EditorState::process_event(const event_type& event)
      {
        if (event.type == sf::Event::KeyPressed)
        {
          switch (event.key.code)
          {
          case sf::Keyboard::Add:
            editor_scene_.move_camera({ 0.0f, 0.0f, -2.0f });
            break;

          case sf::Keyboard::Subtract:
            editor_scene_.move_camera({ 0.0f, 0.0f, 2.0f });
            break;

          case sf::Keyboard::Left:
            editor_scene_.move_camera_2d({ -5.0f, 0.0f });
            break;

          case sf::Keyboard::Right:
            editor_scene_.move_camera_2d({ 5.0f, 0.0f });
            break;

          case sf::Keyboard::Up:
            editor_scene_.move_camera_2d({ 0.0f, 5.0f });
            break;

          case sf::Keyboard::Down:
            editor_scene_.move_camera_2d({ 0.0f, -5.0f });
            break;
          }
        }

        event_queue_.push_back(event);

        menu::update_input_state(input_state_, event);
      }

      void EditorState::active_tool_changed(Tool previous, Tool current)
      {
        auto tool_pointer = [=]() -> EditorTool*
        {
          switch (current)
          {
          case Tool::Path:
            return &path_tool_;

          case Tool::Terrain:
            return &terrain_tool_;

          case Tool::Elevation:
            return &elevation_tool_;

          default:
            return nullptr;
          }
        }();
        
        if (tool_pointer)
        {
          active_tool_ = tool_pointer;
        }
      }

      void EditorState::active_mode_changed(std::size_t previous, std::size_t current)
      {
        if (active_tool_)
        {
          active_tool_->set_active_mode(current);
        }        
      }
    }
  }
}