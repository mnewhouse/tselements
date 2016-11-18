/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace client
  {
    class LocalMessageDispatcher;

    template <typename MessageConveyor>
    class MessageConveyor;
  }

  namespace server
  {
    using LocalConveyor = client::MessageConveyor<client::LocalMessageDispatcher>;
  }
}
