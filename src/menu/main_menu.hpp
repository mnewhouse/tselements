/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef MAIN_MENU_HPP_76234571
#define MAIN_MENU_HPP_76234571

#include <cstdint>
#include <memory>

namespace ts
{
  namespace resources
  {
    class ResourceStore;
  }

  namespace gui
  {
    class Renderer;
    struct RenderState;
    struct InputState;
  }

  namespace menu
  {
    class MainMenu
    {
    public:
      explicit MainMenu(resources::ResourceStore* resource_store);
      ~MainMenu();

      void update(const gui::InputState& input_state, std::uint32_t frame_duration);
      void render(const gui::Renderer& renderer, const gui::RenderState& render_state) const;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}

#endif