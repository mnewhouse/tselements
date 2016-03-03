/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CLIENT_HPP_894531234
#define CLIENT_HPP_894531234

#include <memory>

namespace ts
{
  namespace client
  {
    class MessageConveyor;

    // The Client class encompasses all the functionality that has to do with a client-side cup.
    class Client
    {
    public:
      Client();
      ~Client();

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}

#endif