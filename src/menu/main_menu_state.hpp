/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef MAIN_MENU_STATE_HPP_4819324581
#define MAIN_MENU_STATE_HPP_4819324581

#include "main_menu.hpp"

#include "game/game_state.hpp"

namespace ts
{
  namespace menu
  {
    class MainMenuState
      : public game::GameState
    {
    public:
      MainMenuState(const game::GameContext& game_context);

      virtual void update(const update_context& context) override;
      virtual void render(const render_context& context) const override;

    private:
      MainMenu main_menu_;
    }
  }
}

#endif