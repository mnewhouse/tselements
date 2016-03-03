/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "remote_client.hpp"

namespace ts
{
  namespace server
  {
    RemoteClient::RemoteClient(local_client_t)
      : type_(ClientType::Local)
    {
    }

    RemoteClient::RemoteClient(all_clients_t)
      : type_(ClientType::All)
    {
    }

    RemoteClient::RemoteClient()
      : type_(ClientType::All)
    {
    }

    ClientType RemoteClient::type() const
    {
      return type_;
    }
  }
}