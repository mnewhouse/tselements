/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "editor_state.hpp"

#include "game/loading_thread.hpp"

#include "graphics/render_window.hpp"

#include "resources/resource_store.hpp"
#include "resources/track_loader.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_guards.hpp"
#include "imgui/imgui_default_style.hpp"

namespace ts
{
  namespace editor
  {
    namespace detail
    {
      void zoom_camera(scene::Camera& camera, int delta, double zoom_factor, double min, double max,
                       Vector2<double> center_offset)
      {
        auto inc = delta < 0 ? 1 : -1;
        auto zoom = camera.zoom_level();
        auto old_zoom = zoom;
        if (delta < 0) zoom_factor = 1.0 / zoom_factor;

        while (delta != 0)
        {
          delta += inc;
          zoom *= zoom_factor;
        }

        zoom = std::max(std::min(zoom, max), min);
        camera.set_zoom_level(zoom);

        auto offset = (center_offset / old_zoom) - (center_offset / zoom);
        camera.set_position(camera.position() + offset);
      }

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
      InterfaceState(ToolType::None, 0),
      active_tool_(),
      path_tool_(),
      view_port_({}),
      action_history_(64)
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
    }

    void EditorState::update_loading_state()
    {
      if (loading_future_.valid())
      {
        if (loading_future_.wait_for(std::chrono::seconds(0)) != std::future_status::timeout)
        {
          editor_scene_ = loading_future_.get();

          set_active_state(StateId::Editor);
        }
      }

      if (test_loading_future_.valid() && editor_scene_)
      {
        if (test_loading_future_.wait_for(std::chrono::seconds(0)) != std::future_status::timeout)
        {
          test_state_ = test_loading_future_.get();
          set_active_state(StateId::Test);

          test_state_->launch(context(), editor_scene_->steal_render_scene());
        }
      }
    }

    void EditorState::on_activate()
    {
      if (test_state_ && editor_scene_)
      {
        editor_scene_->adopt_render_scene(test_state_->steal_render_scene());
        test_state_ = nullptr;
      }
    }

    void EditorState::update(const update_context& ctx)
    {
      update_loading_state();    

      auto screen_size = context().render_window->size();
      auto old_state = active_state();

      auto window_size = vector2_cast<float>(screen_size);

      auto& io = ImGui::GetIO();

      imgui::StyleGuard style;
      imgui::ColorGuard color;

      auto has_scene = editor_scene_ != nullptr;

      float menu_bar_height = 0.0f;
      if (ImGui::BeginMainMenuBar())
      {
        if (ImGui::BeginMenu("File"))
        {
          ImGui::MenuItem("New...", "Ctrl+N");
          ImGui::MenuItem("Open...", "Ctrl+O");
          ImGui::Separator();
          ImGui::MenuItem("Save", "Ctrl+S");
          ImGui::MenuItem("Save As...");
          ImGui::Separator();
          ImGui::MenuItem("Exit", "Alt+F4");
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools", has_scene))
        {
            struct Item
            {
              EditorTool* tool;
              ToolType type;
              const char* shortcut;
            };

            Item tools[] =
            {
              { &path_tool_, ToolType::Path, "P" },
              { &tile_tool_, ToolType::Tiles, "T" }
            };

            for (auto item : tools)
            {
              if (ImGui::MenuItem(item.tool->tool_name(), item.shortcut, active_tool_ == item.tool))
              {
                set_active_tool(item.type);
              }
            }

          ImGui::EndMenu();
        }

        EditorTool::mode_name_range mode_names(nullptr, nullptr);
        if (active_tool_) mode_names = active_tool_->mode_names();


        auto mode = active_mode();
        if (ImGui::BeginMenu("Modes", has_scene && !mode_names.empty()))
        {
          std::uint32_t index = 0;
          for (auto mode_name : mode_names)
          {
            if (ImGui::MenuItem(mode_name, std::to_string(index).c_str(), mode == index))
            {
              set_active_mode(index);
            }

            ++index;
          }

          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Test", has_scene))
        {
          if (ImGui::MenuItem("Setup...", "F5"))
          {

          }

          if (ImGui::MenuItem("Test!", "F6"))
          {
            async_load_test_state();
          }

          ImGui::EndMenu();
        }

        menu_bar_height = ImGui::GetItemBoxMax().y;
        ImGui::EndMainMenuBar();
      }

      color.push(ImGuiCol_WindowBg, ImColor(0, 0, 0, 0));
      style.push(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
      style.push(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

      {
        ImVec2 pos(0, static_cast<float>(menu_bar_height));
        ImVec2 size(static_cast<float>(screen_size.x), static_cast<float>(screen_size.y - menu_bar_height));

        ImGui::SetNextWindowPos(pos, ImGuiSetCond_Always);
        ImGui::SetNextWindowSize(size, ImGuiSetCond_Always);
      }

      ImGui::Begin("editor_window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

      if (editor_scene_)
      {
        editor_scene_interface();
      }

      ImGui::End(); // End main window
    }

    void EditorState::editor_scene_interface()
    {
      imgui::ColorGuard color;
      imgui::StyleGuard style;

      auto& io = ImGui::GetIO();
      auto& style_vars = ImGui::GetStyle();
            
      auto world_size = vector2_cast<double>(editor_scene_->track().size());

      tool_pane_windows();

      ImGui::SameLine();

      auto& camera = view_port_.camera();
      detail::clamp_camera(view_port_, world_size);

      auto old_camera_state = camera;

      auto zoom_level = camera.zoom_level();
      auto camera_position = camera.position();

      auto content_size = world_size * zoom_level;
      
      color.push(ImGuiCol_ChildWindowBg, ImColor(0, 0, 0, 0));
      style.push(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
      style.push(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
      ImGui::SetNextWindowContentSize(ImVec2(content_size.x, content_size.y));
      ImGui::BeginChild("canvas_window", ImVec2(0, 0), false,
                        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

      // Calculate the client area.
      auto window_pos = ImGui::GetWindowPos();
      auto window_size = ImGui::GetWindowSize(), canvas_size = window_size;

      // If the content size exceeds the canvas size, the scrollbars are visible, so we need to subtract their size.
      if (content_size.x - canvas_size.x > 0.5) canvas_size.y -= style_vars.ScrollbarSize;
      if (content_size.y - canvas_size.y > 0.5) canvas_size.x -= style_vars.ScrollbarSize;

      IntRect canvas_rect(static_cast<int>(window_pos.x),
                          static_cast<int>(window_pos.y),
                          static_cast<int>(canvas_size.x),
                          static_cast<int>(canvas_size.y));

      view_port_.set_screen_rect(canvas_rect);

      auto scroll_state = make_vector2(ImGui::GetScrollX(), ImGui::GetScrollY());
      auto mouse_pos = ImGui::GetMousePos();

      // If the scroll state changed from the previous frame, adapt the camera position to said scroll state.
      if (std::abs(scroll_state.x - canvas_scroll_state_.x) > 0.5f ||
          std::abs(scroll_state.y - canvas_scroll_state_.y) > 0.5f)
      {
        auto new_position = (scroll_state + make_vector2(canvas_size.x, canvas_size.y) * 0.5) / zoom_level;

        camera.set_position(new_position);
        detail::clamp_camera(view_port_, world_size);

        // And make sure to remember this state.
        old_camera_state = camera;
      }

      if (ImGui::IsMouseClicked(1) && ImGui::IsWindowHovered())
      {
        canvas_drag_state_ = true;
        ImGui::SetWindowFocus();
      }

      else if (ImGui::IsMouseReleased(1))
      {
        canvas_drag_state_ = false;
      }
      
      // If we are dragging the camera with the right mouse button
      if (canvas_drag_state_ && ImGui::IsMouseDragging(1) && (mouse_pos.x >= 0 || mouse_pos.y >= 0))
      {
        auto drag_delta = ImGui::GetMouseDragDelta(1);
        if (drag_delta.x != 0 || drag_delta.y != 0)
        {
          ImGui::ResetMouseDragDelta(1);
        }

        camera_position -= make_vector2(drag_delta.x, drag_delta.y) / zoom_level;
        camera.set_position(camera_position);

        detail::clamp_camera(view_port_, world_size);
      }

      // If the window is focused and we are scrolling, adjust the camera zoom.
      if (ImGui::IsWindowFocused() && io.MouseWheel != 0)
      {
        auto min_zoom = std::min(canvas_size.x / world_size.x,
                                 canvas_size.y / world_size.y);

        auto mouse_pos = ImGui::GetMousePos();
        auto window_pos = ImGui::GetWindowPos();
        
        auto window_center = make_vector2(window_pos.x, window_pos.y) + make_vector2(canvas_size.x, canvas_size.y) * 0.5;
        auto relative_mouse_pos = (make_vector2(mouse_pos.x, mouse_pos.y) - window_center) / zoom_level;

        detail::zoom_camera(camera, io.MouseWheel, 1.08, min_zoom, 8.0, relative_mouse_pos);
        detail::clamp_camera(view_port_, world_size);

        zoom_level = camera.zoom_level();
        camera_position = camera.position();

        content_size = world_size * zoom_level;
      }

      ImGui::EndChild();

      // Now, recreate the window with its potentially new content size.
      ImGui::SetNextWindowContentSize(ImVec2(content_size.x, content_size.y));
      ImGui::BeginChild("canvas_window");

      // If the camera state was altered, adapt the scroll bar states to match the camera.
      if (camera_snapshot_.zoom_level() != zoom_level ||
          camera_snapshot_.position() != camera_position)
      {
        scroll_state = vector2_cast<float>(camera_position * zoom_level - make_vector2(canvas_size.x, canvas_size.y) * 0.5);

        ImGui::SetScrollX(scroll_state.x);
        ImGui::SetScrollY(scroll_state.y);
      }

      camera_snapshot_ = old_camera_state;
      canvas_scroll_state_ = scroll_state;

      if (active_tool_)
      {
        active_tool_->update_canvas_interface(make_context());
      }
      
      ImGui::EndChild();
    }

    void EditorState::tool_pane_windows()
    {
      imgui::StyleGuard style;
      imgui::ColorGuard color;

      color.push(ImGuiCol_ChildWindowBg, imgui::default_style_colors[ImGuiCol_MenuBarBg]);
      ImGui::BeginChild("tool_pane", { 200.f, 0 }, true, ImGuiWindowFlags_NoTitleBar);

      auto tool_name = "Tool Info";
      if (active_tool_) tool_name = active_tool_->tool_name();

      if (ImGui::CollapsingHeader(tool_name, "tool_info", true, true))
      {
        if (active_tool_ && editor_scene_)
        {
          active_tool_->update_tool_info(make_context());
        }        
      }

      if (ImGui::CollapsingHeader("History", ImGuiTreeNodeFlags_DefaultOpen))
      {
        history_window();
      }
       
      if (ImGui::CollapsingHeader("Layers", ImGuiTreeNodeFlags_DefaultOpen))
      {
        layers_window();
      }

      ImGui::EndChild();
    }

    void EditorState::history_window()
    {
      auto window_size = ImGui::GetWindowSize();
      imgui::StyleGuard style;
      imgui::ColorGuard color;

      style.push(ImGuiStyleVar_WindowPadding, ImVec2(3, 3));
      style.push(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
      ImGui::BeginChild("window_frame", ImVec2(window_size.x, 200), true);
      color.push(ImGuiCol_FrameBg, ImColor(0.45f, 0.45f, 0.5f, 1.0f));

      ImGui::ListBoxHeader("", ImVec2(window_size.x - 6, 200 - 6));

      char format_buffer[32];
      auto action_label = [&](std::size_t index)
      {
        std::sprintf(format_buffer, "##action_%llu", index);
        return +format_buffer;
      };

      auto stack_size = action_history_.stack_size();
      auto current_index = action_history_.current_index();
      std::size_t index = 0;

      // Draw an item for all the actions that can be undone.
      for (; index != current_index; ++index)
      {        
        if (ImGui::Selectable(action_label(index), false))
        {
          // Undo all the actions that happened after this point
          action_history_.undo(current_index - index);
        }

        ImGui::SameLine();
        ImGui::TextUnformatted(action_history_.action_description(index).c_str());
        ImGui::Separator();
      }

      color.push(ImGuiCol_Text, ImColor(0.65f, 0.65f, 0.65f, 1.0f));

      // Draw an item for the currently active action
      if (index < stack_size)
      {
        // This is not really a selectable so much, because it would have no effect
        ImGui::Selectable(action_label(index), true);
        ImGui::SameLine();
        ImGui::TextUnformatted(action_history_.action_description(index).c_str());
        ImGui::Separator();
        ++index;
      }

      // And finally, the remaining items (that have been undone, and can be redone)
      for (; index != stack_size; ++index)
      {
        if (ImGui::Selectable(action_label(index), false))
        {
          action_history_.redo(index - current_index);
        }

        ImGui::SameLine();
        ImGui::TextUnformatted(action_history_.action_description(index).c_str());
        ImGui::Separator();
      }

      if (ImGui::Selectable(action_label(stack_size), current_index == stack_size))
      {
        action_history_.redo(stack_size - current_index);
      }

      ImGui::SameLine();
      ImGui::TextUnformatted("Current");

      ImGui::ListBoxFooter();
      ImGui::EndChild();
    }

    void EditorState::layers_window()
    {
      
    }

    void EditorState::process_event(const event_type& event)
    {
      using Key = sf::Keyboard::Key;

      if (event.type == sf::Event::KeyPressed)
      {
        switch (event.key.code)
        {
        case Key::BackSpace:
          if (active_tool_) active_tool_->delete_last(make_context());
          break;

        case Key::Delete:
          if (active_tool_) active_tool_->delete_selected(make_context());
          break;

        case Key::F5:
          // Test setup;
          break;

        case Key::F6:
          async_load_test_state();
          break;

        case Key::P:
          if (editor_scene_) set_active_tool(ToolType::Path);
          break;

        case Key::T:
          if (editor_scene_) set_active_tool(ToolType::Tiles);
          break;

        case Key::Z:
          if (event.key.control) action_history_.undo();
          break;

        case Key::Y:
          if (event.key.control) action_history_.redo();
          break;

        default:
          break;
        }
      }
    }

    void EditorState::active_tool_changed(ToolType previous, ToolType current)
    {
      auto tool_pointer = [=]() -> EditorTool*
      {
        switch (current)
        {
        case ToolType::Path:
         return &path_tool_;

        case ToolType::Tiles:
          return &tile_tool_;

        default:
          return nullptr;
        }
      }();

      if (tool_pointer)
      {
        active_tool_ = tool_pointer;
      }
    }

    void EditorState::active_mode_changed(std::uint32_t previous, std::uint32_t current)
    {
      if (active_tool_)
      {
        active_tool_->set_active_mode(current);
      }
    }

    EditorContext EditorState::make_context()
    {
      return
      {
        *editor_scene_,
        static_cast<InterfaceState&>(*this),
        working_state_,
        action_history_,
        view_port_,

        vector2_cast<double>(context().render_window->size()),
        vector2_cast<double>(editor_scene_->track().size())
      };
    }
  }
}