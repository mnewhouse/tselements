/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef MAIN_MENU_HPP_76234571
#define MAIN_MENU_HPP_76234571

#include <cstdint>

namespace ts
{
  namespace menu
  {
    class MainMenu
    {
    public:
      void update(std::uint32_t frame_duration);
      void render();

    private:

    };
  }
}

#endif