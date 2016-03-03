/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef MESSAGE_TRAITS_HPP_581295823
#define MESSAGE_TRAITS_HPP_581295823

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


#endif