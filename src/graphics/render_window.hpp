/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "utility/vector2.hpp"
#include "utility/color.hpp"

#include <SFML/Window/Event.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>

namespace ts
{
  namespace graphics
  {
    enum class WindowMode
    {
      Windowed,
      FullScreen,
      Borderless
    };

    // The RenderWindow represents the game's window.
    class RenderWindow
    {
    public:
      RenderWindow(const char* title, std::int32_t width, std::int32_t height, WindowMode window_mode);
      ~RenderWindow();

      void set_framerate_limit(std::uint32_t limit);
      void set_vsync_enabled(bool enable);

      void clear(Colorf color = Colorf(0.0f, 0.0f, 0.0f, 1.0f));
      void display();
      void activate();

      Vector2i size() const;

      bool has_focus() const;
      bool poll_event(sf::Event& event);

      Vector2i mouse_position() const;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}
