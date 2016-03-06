/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "render_window.hpp"
#include "gl_context.hpp"

#include <string>
#include <stdexcept>

#include <SFML/Window.hpp>
#include <SFML/Window/ContextSettings.hpp>


namespace ts
{
  namespace graphics
  {
    struct RenderWindow::Impl
      : public sf::Window
    {
      Impl(const char* title, std::uint32_t width, std::uint32_t height, WindowMode window_mode)
      {
        std::uint32_t style = sf::Style::Titlebar;
        if (window_mode == WindowMode::FullScreen) style = sf::Style::Fullscreen;

        sf::ContextSettings context_settings;
        context_settings.majorVersion = gl_version::major;
        context_settings.minorVersion = gl_version::minor;

        sf::Window::create(sf::VideoMode(width, height), title, style, context_settings);
        setVerticalSyncEnabled(true);
      }
    };

    RenderWindow::RenderWindow(const char* title, std::uint32_t width, std::uint32_t height, WindowMode window_mode)
      : impl_(std::make_unique<Impl>(title, width, height, window_mode))
    {
    }

    RenderWindow::~RenderWindow()
    {
    }

    Vector2u RenderWindow::size() const
    {
      auto result = impl_->getSize();
      return Vector2u(result.x, result.y);
    }

    void RenderWindow::display()
    {
      impl_->display();
    }

    void RenderWindow::activate()
    {
      impl_->setActive(true);
    }

    bool RenderWindow::poll_event(sf::Event& event)
    {
      return impl_->pollEvent(event);
    }
  }
}

