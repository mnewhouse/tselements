/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CUP_STATE_HPP_44148293
#define CUP_STATE_HPP_44148293

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

#endif