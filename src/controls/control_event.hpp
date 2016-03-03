/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CONTROL_EVENT_HPP_4841298
#define CONTROL_EVENT_HPP_4841298

#include "control.hpp"

namespace ts
{
  namespace controls
  {
    struct ControlEvent
    {
      Control control;

      bool state;
    };
  }
}

#endif