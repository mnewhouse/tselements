/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "track_editor_state.hpp"

#include "graphics/render_window.hpp"

#include "user_interface/gui_context.hpp"

#include "menu/update_input_state.hpp"

#include "resources/resource_store.hpp"
#include "resources/track_loader.hpp"

namespace ts
{
  namespace editor
  {
    namespace track
    {
      namespace detail
      {
        void clamp_camera(scene::Viewport& viewport, Vector2<double> world_size)
        {
          auto screen_rect = viewport.screen_rect();
          auto screen_size = vector2_cast<double>(make_vector2(screen_rect.width, screen_rect.height));

          auto position = scene::compute_camera_center(viewport.camera(), world_size, screen_size, 0.0);
          viewport.camera().set_position(position);          
        }
      }

      EditorState::EditorState(const game_context& ctx)
        : GameState(ctx),
          InterfaceState(Tool::None, 0),
          track_editor_menu_(&ctx.resource_store->font_library()),
          active_tool_(nullptr),
          view_port_({})
      {
        if (active_tool_) active_tool_->set_active_mode(active_mode());
      }

      EditorState::EditorState(const game_context& ctx, const std::string& file_name)
        : EditorState(ctx)
      {
        async_load_track(file_name);
      }

      void EditorState::async_load_track(const std::string& file_name)
      {
        loading_future_ = context().loading_thread->async_task([=]()
        {
          resources::TrackLoader track_loader;
          track_loader.load_from_file(file_name);
          return std::make_unique<EditorScene>(track_loader.get_result());
        });
      }

      void EditorState::async_load_test_state()
      {
        test_loading_future_ = context().loading_thread->async_task([=]()
        {          
          return std::make_unique<TestState>(editor_scene_->track(), context().resource_store->settings());
        });
      }

      void EditorState::render(const render_context& ctx) const
      {
        if (editor_scene_) editor_scene_->render(view_port_, ctx.screen_size, ctx.frame_progress);

        if (active_tool_) active_tool_->render();

        auto game_context = context();
        const auto& gui_renderer = game_context.gui_context->renderer();

        gui::RenderState render_state;
        render_state.screen_size = vector2_cast<float>(game_context.render_window->size());
        render_state.translation = { 0.0f, 0.0f };

        gui::draw(gui_renderer, gui_geometry_, render_state);
      }

      void EditorState::poll_loading_state()
      {
        if (loading_future_.valid())
        {
          if (loading_future_.wait_for(std::chrono::seconds(0)) != std::future_status::timeout)
          {
            editor_scene_ = loading_future_.get();
            set_active_state(StateId::Editor);
          }
        }

        if (test_loading_future_.valid())
        {
          if (test_loading_future_.wait_for(std::chrono::seconds(0)) != std::future_status::timeout)
          {
            test_state_ = test_loading_future_.get();
            set_active_state(StateId::Test);

            test_state_->launch(context(), editor_scene_->steal_render_scene());
          }
        }
      }

      void EditorState::update(const update_context& ctx)
      {
        poll_loading_state();

        // Update GUI
        clear(gui_geometry_);

        auto screen_size = context().render_window->size();

        auto old_state = active_state();

        auto menu_update_state = track_editor_menu_.update(*this, screen_size, input_state_, gui_geometry_);
        auto menu_area = menu_update_state.bounding_box;

        {
          std::int32_t bottom = menu_area.bottom(), screen_height = screen_size.y;
          view_port_.set_screen_rect({ 0, bottom, menu_area.width, screen_height - bottom });
        }

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

        if (editor_scene_)
        {
          auto& camera = view_port_.camera();

          auto world_size = vector2_cast<double>(editor_scene_->track().size());
          detail::clamp_camera(view_port_, world_size);

          if (has_focus && click_state(input_state_, gui::MouseButton::Right))
          {
            auto delta = vector2_cast<double>(input_state_.mouse_position - input_state_.old_mouse_position);

            
            auto zoom = camera.zoom_level();
            camera.set_position(camera.position() - delta / zoom);
            detail::clamp_camera(view_port_, world_size);

            // TODO: Handle rotation properly
          }

          if (has_focus && input_state_.mouse_wheel_delta != 0)
          {
            auto delta = input_state_.mouse_wheel_delta;
            auto inc = delta < 0 ? 1 : -1;
            auto zoom = camera.zoom_level();
            auto zoom_factor = delta < 0 ? 1.0 / 1.05 : 1.05;

            while (delta != 0)
            {
              delta += inc;
              zoom *= zoom_factor;
            }

            zoom = std::max(std::min(zoom, 10.0), 0.1);

            camera.set_zoom_level(zoom);
            detail::clamp_camera(view_port_, world_size);
          }
        }

        // And empty the event queue.
        event_queue_.clear();

        menu::update_old_input_state(input_state_);

        auto new_state = active_state();
        if (new_state != old_state)
        {
          if (new_state == StateId::Test)
          {
            // Start loading test components
            async_load_test_state();
          }
        }
      }

      void EditorState::process_event(const event_type& event)
      {
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
           // return &path_tool_;

          case Tool::Terrain:
           // return &terrain_tool_;

          case Tool::Elevation:
           // return &elevation_tool_;

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