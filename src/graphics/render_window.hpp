/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef WINDOW_HPP_58912
#define WINDOW_HPP_58912

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
      FullScreenDesktop
    };

    // The RenderWindow models the game's window. 
    // It exposes a very minimalistic interface.
    class RenderWindow
    {
    public:
      RenderWindow(const char* title, std::uint32_t width, std::uint32_t height, WindowMode window_mode);
      ~RenderWindow();

      void set_framerate_limit(std::uint32_t limit);

      void clear(Colorf color = Colorf(0.0f, 0.0f, 0.0f, 1.0f));
      void display();
      void activate();

      Vector2u size() const;

      bool poll_event(sf::Event& event);      

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }

}

#endif