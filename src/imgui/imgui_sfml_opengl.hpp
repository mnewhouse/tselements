/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <SFML/Window/Event.hpp>

#include <array>
#include <memory>

namespace ts
{
  namespace graphics
  {
    class RenderWindow;
  }

  namespace imgui
  {
    class Context
    {
    public:
      explicit Context(graphics::RenderWindow* render_window);

      void process_event(const sf::Event& event);
      void new_frame(std::int32_t frame_duration);

      void render();

    private:
      graphics::RenderWindow* render_window_;

      class Renderer;
      struct RendererDeleter
      {
        void operator()(void*) const;
      };

      std::unique_ptr<Renderer, RendererDeleter> renderer_;

      std::array<bool, 5> mouse_press_events_ = {};
      float mouse_wheel_delta_ = 0.0f;
    };
  }
}
