/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "imgui_sfml_opengl.hpp"
#include "imgui.h"

#include "graphics/gl_check.hpp"
#include "graphics/gl_scissor_box.hpp"
#include "graphics/shader.hpp"
#include "graphics/buffer.hpp"
#include "graphics/texture.hpp"
#include "graphics/render_window.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <GL/glew.h>
#include <GL/GL.h>

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>

namespace ts
{
  namespace imgui
  {
    static void setup_key_map()
    {
      auto& key_map = ImGui::GetIO().KeyMap;

      using sf::Keyboard;
      key_map[ImGuiKey_Tab] = Keyboard::Tab;                   // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
      key_map[ImGuiKey_LeftArrow] = Keyboard::Left;
      key_map[ImGuiKey_RightArrow] = Keyboard::Right;
      key_map[ImGuiKey_UpArrow] = Keyboard::Up;
      key_map[ImGuiKey_DownArrow] = Keyboard::Down;
      key_map[ImGuiKey_PageUp] = Keyboard::PageUp;
      key_map[ImGuiKey_PageDown] = Keyboard::PageDown;
      key_map[ImGuiKey_Home] = Keyboard::Home;
      key_map[ImGuiKey_End] = Keyboard::End;
      key_map[ImGuiKey_Delete] = Keyboard::Delete;
      key_map[ImGuiKey_Backspace] = Keyboard::BackSpace;
      key_map[ImGuiKey_Enter] = Keyboard::Return;
      key_map[ImGuiKey_Escape] = Keyboard::Escape;
      key_map[ImGuiKey_A] = Keyboard::A;
      key_map[ImGuiKey_C] = Keyboard::C;
      key_map[ImGuiKey_V] = Keyboard::V;
      key_map[ImGuiKey_X] = Keyboard::X;
      key_map[ImGuiKey_Y] = Keyboard::Y;
      key_map[ImGuiKey_Z] = Keyboard::Z;
    }

    static graphics::Texture create_dummy_texture()
    {
      sf::Image dummy_image;
      dummy_image.create(2, 2, sf::Color::White);

      return graphics::create_texture_from_image(dummy_image);
    }


    class Context::Renderer
    {
    public:
      Renderer();

      void render(ImDrawData* draw_data) const;

      void initialize_fonts();

    private:
      graphics::ShaderProgram create_shader_program();

      void assign_vao_state();      

      graphics::ShaderProgram shader_program_;
      graphics::Buffer vertex_buffer_;
      graphics::Buffer index_buffer_;
      graphics::VertexArray vertex_array_;

      mutable std::size_t index_buffer_size_ = 0;
      mutable std::size_t vertex_buffer_size_ = 0;      
      mutable std::size_t index_buffer_write_offset_ = 0;
      mutable std::size_t vertex_buffer_write_offset_ = 0;

      std::vector<graphics::Texture> textures_;

      std::uint32_t projection_matrix_location_;
      std::uint32_t texture_sampler_location_;
    };

    void Context::RendererDeleter::operator()(void* ptr) const
    {
      delete static_cast<Renderer*>(ptr);
    }

    Context::Renderer::Renderer()
      : vertex_buffer_(graphics::create_buffer()),
        index_buffer_(graphics::create_buffer()),
        vertex_array_(graphics::create_vertex_array()),
        shader_program_(create_shader_program())
    {
      textures_.push_back(create_dummy_texture());

      projection_matrix_location_ = glCheck(glGetUniformLocation(shader_program_.get(), "u_projectionMatrix"));
      texture_sampler_location_ = glCheck(glGetUniformLocation(shader_program_.get(), "u_textureSampler"));

      assign_vao_state();
    }

    void Context::Renderer::assign_vao_state()
    {
      glCheck(glBindVertexArray(vertex_array_.get()));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.get()));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_.get()));

      auto position_location = glCheck(glGetAttribLocation(shader_program_.get(), "in_position"));
      auto tex_coord_location = glCheck(glGetAttribLocation(shader_program_.get(), "in_texCoord"));
      auto color_location = glCheck(glGetAttribLocation(shader_program_.get(), "in_color"));

      glCheck(glEnableVertexAttribArray(position_location));
      glCheck(glEnableVertexAttribArray(tex_coord_location));
      glCheck(glEnableVertexAttribArray(color_location));

      glCheck(glVertexAttribPointer(position_location, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
                                    reinterpret_cast<const void*>(offsetof(ImDrawVert, pos))));

      glCheck(glVertexAttribPointer(tex_coord_location, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
                                    reinterpret_cast<const void*>(offsetof(ImDrawVert, uv))));

      glCheck(glVertexAttribPointer(color_location, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert),
                                    reinterpret_cast<const void*>(offsetof(ImDrawVert, col))));

      vertex_buffer_size_ = graphics::next_power_of_two(0x2000 * sizeof(ImDrawVert));
      index_buffer_size_ = graphics::next_power_of_two(0x10000 * sizeof(ImDrawIdx));

      glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size_, nullptr, GL_DYNAMIC_DRAW));
      glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size_, nullptr, GL_DYNAMIC_DRAW));

      glCheck(glBindVertexArray(0));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    graphics::ShaderProgram Context::Renderer::create_shader_program()
    {
      const char* const vertex_shader = R"(
#version 330
uniform mat4 u_projectionMatrix;
in vec2 in_position;
in vec2 in_texCoord;
in vec4 in_color;
out vec2 frag_texCoord;
out vec4 frag_color;
void main()
{
  frag_texCoord = in_texCoord;
  frag_color = in_color;
  gl_Position = u_projectionMatrix * vec4(in_position.xy, 0, 1);
}
)";

      const char* const fragment_shader = R"(
#version 330
uniform sampler2D u_textureSampler;
in vec2 frag_texCoord;
in vec4 frag_color;
out vec4 out_fragColor;
void main()
{
  out_fragColor = frag_color * texture(u_textureSampler, frag_texCoord);
}
)";

      return graphics::create_shader_program(vertex_shader, fragment_shader);
    }

    void Context::Renderer::render(ImDrawData* draw_data) const
    {
      auto& io = ImGui::GetIO();

      auto fb_width = io.DisplaySize.x * io.DisplayFramebufferScale.x;
      auto fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;

      draw_data->ScaleClipRects(io.DisplayFramebufferScale);

      glCheck(glViewport(0, 0, fb_width, fb_height));
      glCheck(glEnable(GL_BLEND));
      glCheck(glBlendEquation(GL_FUNC_ADD));
      glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
      glCheck(glDisable(GL_CULL_FACE));
      glCheck(glDisable(GL_DEPTH_TEST));
      glCheck(glEnable(GL_SCISSOR_TEST));
      glCheck(glEnable(GL_TEXTURE_2D));
      glCheck(glActiveTexture(GL_TEXTURE0));

      auto matrix = glm::ortho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f);
      
      
      glCheck(glUseProgram(shader_program_.get()));
      glCheck(glUniform1i(texture_sampler_location_, 0));
      glCheck(glUniformMatrix4fv(projection_matrix_location_, 1, GL_FALSE, glm::value_ptr(matrix)));

      glCheck(glBindVertexArray(vertex_array_.get()));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_.get());)      
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_.get()));

      std::size_t required_vertex_buffer_size = 0;
      std::size_t required_index_buffer_size = 0;
      for (auto i = 0; i < draw_data->CmdListsCount; ++i)
      {
        auto cmd_list = draw_data->CmdLists[i];
        auto& vertices = cmd_list->VtxBuffer;
        auto& indices = cmd_list->IdxBuffer;
        
        required_vertex_buffer_size += vertices.size();
        required_index_buffer_size += indices.size();
      }

      required_vertex_buffer_size *= sizeof(ImDrawVert);
      required_index_buffer_size *= sizeof(ImDrawIdx);

      auto desired_vertex_buffer_size = graphics::next_power_of_two(required_vertex_buffer_size * 3);
      auto desired_index_buffer_size = graphics::next_power_of_two(required_index_buffer_size * 3);

      if (desired_vertex_buffer_size > vertex_buffer_size_)
      {
        glCheck(glBufferData(GL_ARRAY_BUFFER, desired_vertex_buffer_size, nullptr, GL_STREAM_DRAW));
        vertex_buffer_write_offset_ = 0;
        vertex_buffer_size_ = desired_vertex_buffer_size;
      }

      if (desired_index_buffer_size > index_buffer_size_)
      {
        glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, desired_index_buffer_size, nullptr, GL_STREAM_DRAW));
        index_buffer_write_offset_ = 0;
        index_buffer_size_ = desired_index_buffer_size;
      }

      if (vertex_buffer_write_offset_ + required_vertex_buffer_size > vertex_buffer_size_)
      {        
        vertex_buffer_write_offset_ = 0;
      }

      if (index_buffer_write_offset_ + required_index_buffer_size > index_buffer_size_)
      {       
        index_buffer_write_offset_ = 0;
      }

      auto buffer_offset = reinterpret_cast<ImDrawIdx*>(std::uintptr_t(index_buffer_write_offset_));
      auto base_vertex = vertex_buffer_write_offset_ / sizeof(ImDrawVert);

      
      
      for (auto i = 0; i < draw_data->CmdListsCount; ++i)
      {
        auto cmd_list = draw_data->CmdLists[i];
        const auto& vertices = cmd_list->VtxBuffer;
        const auto& indices = cmd_list->IdxBuffer;

        auto vertex_range_size = vertices.size() * sizeof(vertices.front());
        auto index_range_size = indices.size() * sizeof(indices.front());

        glCheck(glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_write_offset_, vertex_range_size, &vertices.front()));
        glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_write_offset_, index_range_size, &indices.front()));

        vertex_buffer_write_offset_ += vertex_range_size;
        index_buffer_write_offset_ += index_range_size;
        
        for (const auto& command : cmd_list->CmdBuffer)
        {
          if (command.UserCallback)
          {
            command.UserCallback(cmd_list, &command);
          }

          else
          {
            auto tex_id = static_cast<GLuint>(reinterpret_cast<std::uintptr_t>(command.TextureId));
            if (tex_id != 0)
            {
              glCheck(glBindTexture(GL_TEXTURE_2D, tex_id));
            }

            else
            {
              // Bind white dummy texture.              
              glCheck(glBindTexture(GL_TEXTURE_2D, textures_.front().get()));
            }

            auto clip_rect = command.ClipRect;
            glCheck(glScissor(static_cast<GLint>(clip_rect.x),
                              static_cast<GLint>(fb_height - clip_rect.w),
                              static_cast<GLint>(clip_rect.z - clip_rect.x),
                              static_cast<GLint>(clip_rect.w - clip_rect.y)));

            const auto elem_type = sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
            glCheck(glDrawElementsBaseVertex(GL_TRIANGLES, static_cast<GLsizei>(command.ElemCount),
                                             elem_type, buffer_offset, base_vertex));
          }

          buffer_offset += command.ElemCount;
        }

        base_vertex += vertices.size();
      }

      /*
        auto vertex_range_size = cmd_list->VtxBuffer.size() * sizeof(ImDrawVert);
        auto index_range_size = cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx);

        glCheck(glBufferSubData(GL_ARRAY_BUFFER, vertex_buffer_offset, vertex_range_size, 
                                &cmd_list->VtxBuffer.front()));

        glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_offset, index_range_size, 
                                &cmd_list->IdxBuffer.front()));

        std::uintptr_t draw_offset = index_buffer_offset;
        for (const auto& command : cmd_list->CmdBuffer)
        {
          if (command.UserCallback)
          {
            command.UserCallback(cmd_list, &command);
          }

          else
          {
            auto tex_id = static_cast<GLuint>(reinterpret_cast<std::uintptr_t>(command.TextureId));
            glBindTexture(GL_TEXTURE_2D, tex_id);

            auto clip_rect = command.ClipRect;
            glScissor(static_cast<GLint>(clip_rect.x),
                      static_cast<GLint>(fb_height - clip_rect.w),
                      static_cast<GLint>(clip_rect.z - clip_rect.x),
                      static_cast<GLint>(clip_rect.w - clip_rect.y));

            glCheck(glDrawElementsBaseVertex(GL_TRIANGLES, static_cast<GLsizei>(command.ElemCount),
                                             sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                                             reinterpret_cast<const void*>(draw_offset), base_vertex));
          }

          draw_offset += command.ElemCount + sizeof(ImDrawIdx);
        }

        vertex_buffer_offset += vertex_range_size;
        index_buffer_offset += index_range_size;
        base_vertex += cmd_list->VtxBuffer.size();
      }

      for (auto i = 0; i < draw_data->CmdListsCount; ++i)
      {
        auto cmd_list = draw_data->CmdLists[i];
        const ImDrawIdx* index_buffer_offset = nullptr;

        auto vertex_buffer_size = static_cast<GLsizeiptr>(cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
        auto index_buffer_size = static_cast<GLsizeiptr>(cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));

        glCheck(glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size,
                             static_cast<const void*>(&cmd_list->VtxBuffer.front()), GL_STREAM_DRAW));

        glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size,
                             static_cast<const void*>(&cmd_list->IdxBuffer.front()), GL_STREAM_DRAW));


        for (const auto& command : cmd_list->CmdBuffer)
        {
          if (command.UserCallback)
          {
            command.UserCallback(cmd_list, &command);
          }

          else
          {
            auto tex_id = static_cast<GLuint>(reinterpret_cast<std::uintptr_t>(command.TextureId));
            glBindTexture(GL_TEXTURE_2D, tex_id);

            auto clip_rect = command.ClipRect;
            glScissor(static_cast<GLint>(clip_rect.x),
                      static_cast<GLint>(fb_height - clip_rect.w),
                      static_cast<GLint>(clip_rect.z - clip_rect.x),
                      static_cast<GLint>(clip_rect.w - clip_rect.y));

            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command.ElemCount),
                           sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, index_buffer_offset);
          }

          glCheck(glInvalidateBufferData(vertex_buffer_.get()));
          glCheck(glInvalidateBufferData(index_buffer_.get()));

          index_buffer_offset += command.ElemCount;
        }        
      }
      */

      glCheck(glBindTexture(GL_TEXTURE_2D, 0));
      glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
      glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
      glCheck(glBindVertexArray(0));
      glCheck(glUseProgram(0));

      graphics::disable_scissor_box();
    }


    void Context::Renderer::initialize_fonts()
    {
      auto max_size = std::min(graphics::max_texture_size(), 2048);

      auto& io = ImGui::GetIO();
      io.Fonts->TexDesiredWidth = max_size;

      unsigned char* pixels = nullptr;
      int w{}, h{};

      io.Fonts->AddFontFromFileTTF("fonts/segoeuib.ttf", 16.0);
      io.Fonts->GetTexDataAsAlpha8(&pixels, &w, &h);

      GLuint texture{};
      glCheck(glGenTextures(1, &texture));
      glCheck(glBindTexture(GL_TEXTURE_2D, texture));
      glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
      glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
      glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE));
      glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE));
      glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE));
      glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels));
      glCheck(glBindTexture(GL_TEXTURE_2D, 0));

      io.Fonts->SetTexID(reinterpret_cast<void*>(static_cast<std::uintptr_t>(texture)));

      textures_.push_back(graphics::Texture(texture, Vector2u(w, h)));
    }


    Context::Context(graphics::RenderWindow* render_window)
      : render_window_(render_window),
        renderer_(new Renderer())
    {
      setup_key_map();

      renderer_->initialize_fonts();
    }

    void Context::process_event(const sf::Event& event)
    {
      auto& io = ImGui::GetIO();

      if (event.type == sf::Event::MouseButtonPressed)
      {
        auto button_index = [](auto button)
        {
          switch (button)
          {
          case sf::Mouse::Left: return 0;
          case sf::Mouse::Right: return 1;
          case sf::Mouse::Middle: return 2;
          case sf::Mouse::XButton1: return 3;
          case sf::Mouse::XButton2: return 4;
          default: return -1;
          }
        }(event.mouseButton.button);
        
        if (button_index >= 0)
        {
          mouse_press_events_[button_index] = true;
        }
      }

      else if (event.type == sf::Event::MouseWheelScrolled)
      {
        mouse_wheel_delta_ = event.mouseWheelScroll.delta;
      }

      else if (event.type == sf::Event::TextEntered)
      {
        io.AddInputCharacter(event.text.unicode);
      }

      else if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased)
      {
        io.KeysDown[event.key.code] = (event.type == sf::Event::KeyPressed);

        io.KeyAlt = event.key.alt;
        io.KeyCtrl = event.key.control;
        io.KeyShift = event.key.shift;
        io.KeySuper = event.key.system;        
      }
    }

    void Context::new_frame(std::int32_t frame_duration)
    {
      bool has_focus = render_window_->has_focus();

      using sf::Mouse;
      const auto& button_state = Mouse::isButtonPressed;

      auto& io = ImGui::GetIO();
      io.DeltaTime = frame_duration * 0.001f;

      auto window_size = vector2_cast<float>(render_window_->size());      
      io.DisplaySize = ImVec2(window_size.x, window_size.y);
      io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

      auto mouse_pos = render_window_->mouse_position();
      if (mouse_pos.x >= 0 && mouse_pos.y >= 0 && mouse_pos.x < window_size.x && mouse_pos.y < window_size.y || 1)
      {
        io.MousePos = ImVec2(static_cast<float>(mouse_pos.x), static_cast<float>(mouse_pos.y));
      }

      else
      {
        io.MousePos = ImVec2(-1.0f, -1.0f);
      }

      auto button_index = 0;
      for (auto button : { Mouse::Left, Mouse::Right, Mouse::Middle, Mouse::XButton1, Mouse::XButton2 })
      {
        io.MouseDown[button_index] = mouse_press_events_[button_index] || (has_focus && button_state(button));
        ++button_index;
      }
      
      io.MouseWheel = mouse_wheel_delta_;

      mouse_wheel_delta_ = 0;
      mouse_press_events_ = {};
      
      ImGui::NewFrame();
    }

    void Context::render()
    {
      ImGui::Render();

      renderer_->render(ImGui::GetDrawData());
    }
  }
}