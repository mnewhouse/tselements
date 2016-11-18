/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace messages
  {
    template <typename MessageType>
    struct MessageTraits
    {
      using encoder = void;
      using decoder = void;
    };
  }
}
