/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "editor_state.hpp"

#include "game/loading_thread.hpp"

#include "graphics/render_window.hpp"

#include "resources/resource_store.hpp"
#include "resources/track_loader.hpp"
#include "resources/track_saving.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_guards.hpp"
#include "imgui/imgui_default_style.hpp"

#include <boost/range/adaptor/reversed.hpp>

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
      InterfaceState(ModeType::Path, 0),
      active_mode_(&path_mode_),
      view_port_({}),
      action_history_(64)
    {
      if (active_mode_) active_mode_->set_active_tool(active_tool());
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
        return load_test_stage_components(editor_scene_->track(), context().resource_store->settings());
      });
    }

    void EditorState::render(const render_context& ctx) const
    {
      if (editor_scene_)
      {
        auto callback = [this](const sf::Transform& view_matrix)
        {
          if (active_mode_)
          {
            active_mode_->on_canvas_render(make_context(), view_matrix);
          }
        };

        editor_scene_->render(view_port_, ctx.screen_size, ctx.frame_progress, callback);
      }
    }
    void EditorState::update_loading_state()
    {
      if (loading_future_.valid())
      {
        if (loading_future_.wait_for(std::chrono::seconds(0)) != std::future_status::timeout)
        {
          editor_scene_ = loading_future_.get();

          select_default_layer();         
          set_active_state(StateId::Editor);
        }
      }

      if (test_loading_future_.valid() && editor_scene_)
      {
        if (test_loading_future_.wait_for(std::chrono::seconds(0)) != std::future_status::timeout)
        { 
          auto stage_components = test_loading_future_.get();
          adopt_render_scene(stage_components, editor_scene_->steal_render_scene());

          set_active_state(StateId::Test);
          auto test_state = context().state_machine->create_state<TestState>(context(), std::move(stage_components));          
        }
      }
    }

    void EditorState::on_activate()
    {      
      auto test_state = context().state_machine->find_state<TestState>();
      if (test_state && editor_scene_)
      {
        test_state->release_scene(*editor_scene_);       
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
          if (ImGui::MenuItem("Save", "Ctrl+S"))
          {
            resources::save_track(editor_scene_->track());
          }

          ImGui::MenuItem("Save As...");
          ImGui::Separator();
          ImGui::MenuItem("Exit", "Alt+F4");
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit", has_scene))
        {
          if (ImGui::MenuItem("Undo", "Ctrl+Z", false, action_history_.can_undo()))
          {
            action_history_.undo();
          }

          if (ImGui::MenuItem("Redo", "Ctrl+Y", false, action_history_.can_redo()))
          {
            action_history_.redo();
          }

          ImGui::Separator();
          ImGui::MenuItem("Cut", "Ctrl+X", false, false);
          ImGui::MenuItem("Copy", "Ctrl+C", false, false);
          ImGui::MenuItem("Paste", "Ctrl+V", false, false);

          if (ImGui::MenuItem("Delete", "Delete", false, false))
          {
            active_mode_->delete_selected(make_context());
          }

          if (ImGui::MenuItem("Delete Last", "Backspace", false, active_mode_ != nullptr))
          {
            active_mode_->delete_last(make_context());
          }

          ImGui::Separator();
          ImGui::MenuItem("Select All", "Ctrl+A", false, false);

          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View", has_scene))
        {
          ImGui::MenuItem("Reset Zoom");

          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Layers", has_scene))
        {
          auto selected_layer = working_state_.selected_layer();
          auto active = selected_layer != nullptr;

          if (ImGui::MenuItem("Create Layer", "Ctrl+L"))
          {
            open_create_layer_dialog();
          }

          if (ImGui::MenuItem("Delete Layer", "Ctrl+Delete", false, active))
          {
            deactivate_layer(selected_layer);
          }

          if (ImGui::MenuItem("Hide Layer", "H", selected_layer->visible(), active))
          {
            toggle_layer_visibility(selected_layer);
          }

          if (ImGui::MenuItem("Layer Properties", "Ctrl+P", false, active))
          {
            show_layer_properties(selected_layer);
          }

          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Modes", has_scene))
        {
          struct Item
          {
            EditorMode* mode;
            ModeType type;
            const char* shortcut;
          };

          Item modes[] =
          {
            { &path_mode_, ModeType::Path, "P" },
            { &tile_mode_, ModeType::Tiles, "T" },
            { &control_points_mode_, ModeType::ControlPoints, "C" }
          };

          for (auto item : modes)
          {
            if (ImGui::MenuItem(item.mode->mode_name(), item.shortcut, active_mode_ == item.mode))
            {
              set_active_mode(item.type);
            }
          }

          ImGui::EndMenu();
        }

        EditorMode::tool_name_range tool_names(nullptr, nullptr);
        if (active_mode_) tool_names = active_mode_->tool_names();

        auto tool = active_tool();
        if (ImGui::BeginMenu("Tools", has_scene && !tool_names.empty()))
        {
          std::uint32_t index = 0;
          for (auto tool_name : tool_names)
          {
            if (ImGui::MenuItem(tool_name, std::to_string(index + 1).c_str(), tool == index))
            {
              set_active_tool(index);
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
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);

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
      ImGui::SetNextWindowContentSize(ImVec2(static_cast<float>(content_size.x), static_cast<float>(content_size.y)));
      ImGui::BeginChild("canvas_window", ImVec2(0, 0), false,
                        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs |
                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

      // Calculate the client area.
      auto window_pos = ImGui::GetWindowPos();
      auto window_size = ImGui::GetWindowSize(), canvas_size = window_size;

      // If the content size exceeds the canvas size, the scrollbars are visible, so we need to subtract their size.
      if (content_size.x - canvas_size.x > 0.5) canvas_size.y -= style_vars.ScrollbarSize;
      if (content_size.y - canvas_size.y > 0.5) canvas_size.x -= style_vars.ScrollbarSize;

      FloatRect canvas_rect(window_pos.x, window_pos.y, canvas_size.x, canvas_size.y);

      view_port_.set_screen_rect(rect_cast<std::int32_t>(canvas_rect));

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

      canvas_hover_state_ = contains(canvas_rect, { mouse_pos.x, mouse_pos.y });
      canvas_focus_state_ = !ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered() && 
        !ImGui::IsMouseHoveringAnyWindow() && canvas_hover_state_;
      if (canvas_focus_state_)
      {
        if (ImGui::IsMouseClicked(1))
        {
          canvas_drag_state_ = true;
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
        if (io.MouseWheel != 0)
        {
          auto min_zoom = std::min(canvas_size.x / world_size.x,
                                   canvas_size.y / world_size.y);

          auto window_pos = ImGui::GetWindowPos();

          auto window_center = make_vector2(window_pos.x, window_pos.y) + make_vector2(canvas_size.x, canvas_size.y) * 0.5;
          auto relative_mouse_pos = (make_vector2(mouse_pos.x, mouse_pos.y) - window_center) / zoom_level;

          detail::zoom_camera(camera, static_cast<int>(io.MouseWheel), 1.08, min_zoom, 8.0, relative_mouse_pos);
          detail::clamp_camera(view_port_, world_size);

          zoom_level = camera.zoom_level();
          camera_position = camera.position();

          content_size = world_size * zoom_level;
        }
      }

      ImGui::EndChild();

      // Now, recreate the window with its potentially new content size.
      ImGui::SetNextWindowContentSize(ImVec2(static_cast<float>(content_size.x), static_cast<float>(content_size.y)));
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

      if (active_mode_)
      {
        active_mode_->update_canvas_interface(make_context());
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
      if (active_mode_) tool_name = active_mode_->mode_name();

      if (ImGui::CollapsingHeader(tool_name, "tool_info", true, true))
      {
        if (active_mode_ && editor_scene_)
        {
          active_mode_->update_tool_info(make_context());
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

      ImGui::BeginChild("history_frame", ImVec2(window_size.x, 150), true);
      color.push(ImGuiCol_FrameBg, ImColor(0.45f, 0.45f, 0.5f, 1.0f));

      ImGui::ListBoxHeader("", ImVec2(window_size.x - 6, 150 - 6));

      auto stack_size = action_history_.stack_size();
      auto current_index = action_history_.current_index();
      std::size_t index = 0;

      // Draw an item for all the actions that can be undone.
      for (; index != current_index; ++index)
      {        
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::Selectable(action_history_.action_description(index).c_str(), false))
        {
          // Undo all the actions that happened after this point
          action_history_.undo(current_index - index);
        }

        ImGui::PopID();
        ImGui::Separator();
      }

      color.push(ImGuiCol_Text, ImColor(0.65f, 0.65f, 0.65f, 1.0f));

      // Draw an item for the currently active action
      if (index < stack_size)
      {
        ImGui::PushID(static_cast<int>(index));

        // This is not really a selectable so much, because it would have no effect
        ImGui::Selectable(action_history_.action_description(index).c_str(), true);
        ImGui::PopID();

        ImGui::Separator();
        ++index;        
      }

      // And finally, the remaining items (that have been undone, and can be redone)
      for (; index != stack_size; ++index)
      {
        ImGui::PushID(static_cast<int>(index));
        if (ImGui::Selectable(action_history_.action_description(index).c_str(), false))
        {
          action_history_.redo(index - current_index);
        }

        ImGui::PopID();
        ImGui::Separator();
      }

      ImGui::PushID(static_cast<int>(index));
      if (ImGui::Selectable("Latest", current_index == stack_size))
      {
        action_history_.redo(stack_size - current_index);
      }
      ImGui::PopID();

      color.pop();

      ImGui::ListBoxFooter();

      color.pop();
      ImGui::EndChild();
    }

    void EditorState::layers_window()
    {
      auto window_size = ImGui::GetWindowSize();
      imgui::StyleGuard style;
      imgui::ColorGuard color;

      style.push(ImGuiStyleVar_WindowPadding, ImVec2(3, 3));
      ImGui::BeginChild("layers_frame", ImVec2(window_size.x, 150), true);
      color.push(ImGuiCol_FrameBg, ImColor(0.45f, 0.45f, 0.5f, 1.0f));
      color.push(ImGuiCol_CheckMark, ImColor(0.0f, 0.7f, 0.0f, 1.0f));

      auto& track = editor_scene_->track();
      auto selected_layer = working_state_.selected_layer();

      char buffer[40];
      auto label_name = [&](auto* layer)
      {
        std::sprintf(buffer, "##layer_%p", layer);
        return +buffer;
      };

      auto layer_name_popup = [&](auto* layer)
      {
        std::sprintf(buffer, "##layer_name_%p", layer);
        return +buffer;
      };

      auto layer_context_menu_label = [&](auto* layer)
      {
        std::sprintf(buffer, "##layer_actions_%p", layer);
        return +buffer;
      };

      ImVec2 list_box_size(window_size.x - 6, 150 - 6);

      ImGui::ListBoxHeader("", list_box_size);
      for (auto& layer : track.layers() | boost::adaptors::reversed)
      {
        if (ImGui::Selectable(label_name(&layer), &layer == selected_layer, 0, ImVec2(0, 24)))
        {
          working_state_.select_layer(&layer);
        }

        if (ImGui::IsItemActive() && ImGui::IsMouseDragging())
        {
          auto item_pos = ImGui::GetItemRectMin();
          auto mouse_pos = ImGui::GetMousePos();

          auto move_amount = static_cast<std::int32_t>(std::floor((mouse_pos.y - item_pos.y) / 24.0f));
          if (move_amount != 0)
          {
            if (move_amount < 0) track.shift_towards_front(&layer, -move_amount);
            else track.shift_towards_back(&layer, move_amount);
          }
        }

        if (ImGui::BeginPopupContextItem(layer_context_menu_label(&layer), 1))
        {
          bool visible = layer.visible();
          if (ImGui::MenuItem("Visible", nullptr, &visible))
          {
            toggle_layer_visibility(&layer);
          }

          if (ImGui::MenuItem("Rename..."))
          {
            modified_layer_ = &layer;
            layer_rename_window_open_ = true;
          }

          ImGui::EndPopup();
        }

        ImGui::SameLine(10.0f);

        auto color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        if (!layer.visible()) color.w = 0.6f;

        ImGui::PushStyleColor(ImGuiCol_Text, color);

        const auto& layer_name = layer.name();
        ImGui::TextUnformatted(layer_name.data(), layer_name.data() + layer_name.size());

        ImGui::PopStyleColor(1);

        ImGui::Separator();
      }

      ImGui::ListBoxFooter();

      color.pop(2);
      ImGui::EndChild();

      
      if (modified_layer_)
      {
        ImGui::SetNextWindowPosCenter(ImGuiSetCond_Once);
        if (ImGui::Begin("Rename Layer", &layer_rename_window_open_, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse))
        {
          ImGui::SetWindowFocus();
          char buffer[64];
          std::snprintf(buffer, sizeof(buffer), "%s", modified_layer_->name().c_str());
          if (ImGui::InputText("##rename_layer_input", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
          {
            modified_layer_->rename(buffer);
            modified_layer_ = nullptr;
            layer_rename_window_open_ = false;
          }

          ImGui::SetKeyboardFocusHere();
          ImGui::End();
        }
      }
    }

    void EditorState::deactivate_layer(resources::TrackLayer* layer)
    {
      auto ctx = make_context();
      auto action = [=]()
      {
        // We need to let the user think we delete the layer, but we still need to keep it around.
        ctx.scene.deactivate_layer(layer);
      };

      auto undo_action = [=]()
      {
        ctx.scene.activate_layer(layer);
      };

      ctx.action_history.push_action("Delete Layer", action, undo_action);
    }

    void EditorState::toggle_layer_visibility(resources::TrackLayer* layer)
    {   
      auto ctx = make_context();
      auto action = [=]()
      {
        if (!layer->visible())
        {
          ctx.scene.show_layer(layer);
        }        

        else
        {
          ctx.scene.hide_layer(layer);
        }
      };

      ctx.action_history.push_action(layer->visible() ? "Hide Layer" : "Show Layer", action, action);
    }

    void EditorState::show_layer_properties(resources::TrackLayer* layer)
    {

    }

    void EditorState::open_create_layer_dialog()
    {

    }

    void EditorState::process_event(const event_type& event)
    {
      using Key = sf::Keyboard::Key;     

      auto in_focus = !layer_rename_window_open_;
      auto selected_layer = editor_scene_ ? make_context().working_state.selected_layer() : nullptr;

      if (in_focus && event.type == sf::Event::KeyPressed)
      {
        switch (event.key.code)
        {
        case Key::BackSpace:
          if (active_mode_) active_mode_->delete_last(make_context());
          break;

        case Key::Delete:
          if (active_mode_) active_mode_->delete_selected(make_context());
          if (event.key.control && selected_layer) deactivate_layer(selected_layer);
          break;

        case Key::F5:
          // Test setup;
          break;

        case Key::F6:
          async_load_test_state();
          break;

        case Key::S:
          if (event.key.control && editor_scene_)
          {
            resources::save_track(editor_scene_->track());
          }
          break;

        case Key::H:
          if (selected_layer) toggle_layer_visibility(selected_layer);
          break;


        case Key::P:
          if (editor_scene_) set_active_mode(ModeType::Path);
          break;

        case Key::T:
          if (editor_scene_) set_active_mode(ModeType::Tiles);
          break;

        case Key::C:
          if (editor_scene_) set_active_mode(ModeType::ControlPoints);

        case Key::Z:
          if (event.key.control) action_history_.undo();
          break;

        case Key::Y:
          if (event.key.control) action_history_.redo();
          break;

        case Key::L:
          if (event.key.control) open_create_layer_dialog();
          break;

        case Key::Left:
          if (active_mode_) active_mode_->previous(make_context());
          break;

        case Key::Right:
          if (active_mode_) active_mode_->next(make_context());
          break;

        default:
          break;
        }
      }
    }

    void EditorState::select_default_layer()
    {
      resources::TrackLayer* layer = nullptr;
      const auto& layers = editor_scene_->track().layers();
      if (!layers.empty()) layer = &layers.front();

      working_state_.select_layer(layer);
    }

    void EditorState::active_mode_changed(ModeType previous, ModeType current)
    {
      auto mode_pointer = [=]() -> EditorMode*
      {
        switch (current)
        {
        case ModeType::Path:
         return &path_mode_;

        case ModeType::Tiles:
          return &tile_mode_;

        case ModeType::ControlPoints:
          return &control_points_mode_;

        default:
          return nullptr;
        }
      }();

      if (active_mode_ && active_mode_ != mode_pointer)
      {
        active_mode_->deactivate(make_context());
      }

      if (active_mode_ != mode_pointer)
      {
        active_mode_ = mode_pointer;
        if (mode_pointer) mode_pointer->activate(make_context());
      }
    }

    void EditorState::active_tool_changed(std::uint32_t previous, std::uint32_t current)
    {
      if (active_mode_)
      {
        active_mode_->set_active_tool(current);
      }
    }

    template <typename ContextType, typename Self>
    ContextType EditorState::make_context(Self&& self)
    {
      return
      {
        self.canvas_focus_state_,
        *self.editor_scene_,
        self,
        self.working_state_,
        self.action_history_,
        self.view_port_,

        vector2_cast<double>(self.context().render_window->size()),
        vector2_cast<double>(self.editor_scene_->track().size())
      };
    }

    EditorContext EditorState::make_context()
    {
      return make_context<EditorContext>(*this);
    }

    ImmutableEditorContext EditorState::make_context() const
    {
      return make_context<ImmutableEditorContext>(*this);
    }
  }
}