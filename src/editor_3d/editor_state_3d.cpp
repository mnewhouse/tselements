/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "editor_state_3d.hpp"
#include "screen_space_3d.hpp"
#include "editor_context_3d.hpp"

#include "graphics/render_window.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_guards.hpp"
#include "imgui/imgui_default_style.hpp"

namespace ts
{
  namespace editor3d
  {
    namespace
    {
      auto create_test_track()
      {
        Vector2i track_size(1600, 1400);
        auto elevation_map = resources3d::generate_random_elevation_map(track_size, 16,
                                                                        0.0f, 300.0f, 0.5f);

        resources3d::Track track(track_size, std::move(elevation_map));

        return track;
      }
    }

    EditorState::EditorState(const game_context& game_context)
      : game::GameState(game_context),
        view_port_({ 0, 0, 800, 600 }),
        tool_state_(),
        active_tool_(&path_designer_)
    {
      tool_state_.active_tool = ToolType::PathDesigner;     

      editor_scene_ = std::make_unique<EditorScene>(create_test_track());
    }

    void EditorState::render(const render_context& ctx) const
    {
      if (editor_scene_)
      {
        editor_scene_->render(view_port_, ctx.screen_size, ctx.frame_progress);
      }
    }

    void EditorState::update(const update_context& ctx)
    {
      auto screen_size = context().render_window->size();
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

        if (ImGui::BeginMenu("Edit", has_scene))
        {
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View", has_scene))
        {
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Layers", has_scene))
        {
          ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools", has_scene))
        {
          ImGui::EndMenu();
        }

        EditorTool::mode_name_range mode_names(nullptr, nullptr);
        if (active_tool_) mode_names = active_tool_->mode_names();

        auto mode = tool_state_.active_mode_id;
        if (ImGui::BeginMenu("Modes", has_scene && !mode_names.empty()))
        {
          std::uint32_t index = 0;
          for (auto mode_name : mode_names)
          {
            if (ImGui::MenuItem(mode_name, std::to_string(index + 1).c_str(), mode == index))
            {
              tool_state_.active_mode_id = index;
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
          }

          ImGui::EndMenu();
        }

        menu_bar_height = ImGui::GetItemBoxMax().y;
        ImGui::EndMainMenuBar();
      }

      {
        auto y = static_cast<std::int32_t>(menu_bar_height);        
        view_port_.set_screen_rect(IntRect(0, y, screen_size.x, screen_size.y - y));
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

      color.push(ImGuiCol_ChildWindowBg, ImColor(0, 0, 0, 0));
      style.push(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
      style.push(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
      ImGui::BeginChild("canvas_window", ImVec2(0, 0), false,
                        ImGuiWindowFlags_NoScrollWithMouse);

      if (active_tool_)
      {
        active_tool_->update_canvas_interface(make_context());
      }

      ImGui::EndChild();
    }

    EditorContext EditorState::make_context()
    {
      return EditorContext
      {
        *editor_scene_,
        tool_state_,
        working_state_,
        view_port_
      };
    }

    void EditorState::process_event(const event_type& event)
    {      
      using scene3d::Camera;
      if (event.type == sf::Event::KeyPressed)
      {
        using sf::Keyboard;
        auto& camera = view_port_.camera();

        const auto x_offset = make_vector3(10.0f, 0.0f, 0.0f);
        const auto y_offset = make_vector3(0.0f, 10.0f, 0.0f);

        if (event.key.code == Keyboard::Add)
        {          
          camera.set_position(camera.position() - make_vector3(0.0f, 0.0f, 10.0f));
        }

        else if (event.key.code == Keyboard::Subtract)
        {
          camera.set_position(camera.position() + make_vector3(0.0f, 0.0f, 10.0f));
        }

        else if (event.key.code == Keyboard::P && event.key.control)
        {          
          camera.set_view_type(Camera::Perspective);
        }

        else if (event.key.code == Keyboard::O && event.key.control)
        {
          camera.set_view_type(Camera::Orthographic);
        }

        else if (event.key.code == Keyboard::W)
        {          
          camera.set_position(camera.position() - y_offset);

          if (!event.key.shift) camera.set_look_at(camera.look_at() - y_offset);
        }

        else if (event.key.code == Keyboard::S)
        {
          camera.set_position(camera.position() + y_offset);

          if (!event.key.shift) camera.set_look_at(camera.look_at() + y_offset);
        }

        else if (event.key.code == Keyboard::D)
        {
          camera.set_position(camera.position() + x_offset);

          if (!event.key.shift) camera.set_look_at(camera.look_at() + x_offset);
        }

        else if (event.key.code == Keyboard::A)
        {
          camera.set_position(camera.position() - x_offset);

          if (!event.key.shift) camera.set_look_at(camera.look_at() - x_offset);
        }

        else if (event.key.code == Keyboard::X)
        {
          editor_scene_->update_terrain();
        }
      }
    }
  }
}