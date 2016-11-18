/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace cup
  {
    enum class CupState
    {
      Registration,
      Intermission,
      CarSelection,
      PreInitialization,
      Initialization,
      Action,
      End
    };
  }
}
