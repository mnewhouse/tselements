/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "process_priority.hpp"

#ifdef WIN32
#include <Windows.h>
#endif



namespace ts
{
  namespace game
  {
    void elevate_process_priority()
    {      
#ifdef WIN32
      SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif
    }
  }
}



