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

#include <GL/glew.h>


namespace ts
{
  namespace graphics
  {
    struct RenderWindow::Impl
      : public sf::Window
    {
      Impl(const char* title, std::uint32_t width, std::uint32_t height, WindowMode window_mode)
      {
        sf::ContextSettings context_settings;
        context_settings.majorVersion = gl_version::major;
        context_settings.minorVersion = gl_version::minor;
        context_settings.antialiasingLevel = 4;

        std::uint32_t style = sf::Style::Titlebar;
        if (window_mode == WindowMode::FullScreen)
        {
          style = sf::Style::Fullscreen;
        }

        else if (window_mode == WindowMode::FullScreenDesktop)
        {
          const auto& modes = sf::VideoMode::getFullscreenModes();
          style = sf::Style::None;
          width = modes.front().width;
          height = modes.front().height;
        }
        
        sf::Window::create(sf::VideoMode(width, height, 32U), title, style, context_settings);
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

    void RenderWindow::clear(Colorf color)
    {
      glClearColor(color.r, color.g, color.b, color.a);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void RenderWindow::display()
    {
      impl_->display();
      glFinish();
    }

    void RenderWindow::activate()
    {
      impl_->setActive(true);
    }

    bool RenderWindow::poll_event(sf::Event& event)
    {
      return impl_->pollEvent(event);
    }

    void RenderWindow::set_framerate_limit(std::uint32_t limit)
    {
      impl_->setFramerateLimit(limit);
    }
  }
}

