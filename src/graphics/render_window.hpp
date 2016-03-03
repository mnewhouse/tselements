/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef WINDOW_HPP_58912
#define WINDOW_HPP_58912

#include "utility/vector2.hpp"

#include <SFML/Window/Event.hpp>

#include <cstddef>
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

    class RenderWindow
    {
    public:
      RenderWindow(const char* title, int width, int height, WindowMode window_mode);
      ~RenderWindow();

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