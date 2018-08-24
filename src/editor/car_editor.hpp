/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "world/world_message_fwd.hpp"

namespace ts
{
  namespace world
  {
    class Car;
  }

  namespace editor
  {
    class CarEditor
    {
    public:
      bool update(world::messages::CarPropertiesUpdate& msg);

      void activate(const world::Car*);
      void deactivate();

      bool active() const;

    private:
      bool active_ = false;
      const world::Car* car_;
    };
  }
}