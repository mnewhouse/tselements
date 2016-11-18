/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace server
  {
    struct local_client_t {};
    static const local_client_t local_client;

    struct all_clients_t{};
    static const all_clients_t all_clients;

    enum class ClientType
    {
      Local,
      All
    };

    class RemoteClient
    {
    public:
      RemoteClient(local_client_t);
      RemoteClient(all_clients_t);
      RemoteClient();

      ClientType type() const;

    private:
      ClientType type_;
    };

    inline bool operator==(const RemoteClient& a, const RemoteClient& b)
    {
      if (a.type() != b.type()) return false;
      
      // TODO
      return true;
    }

    inline bool operator!=(const RemoteClient& a, const RemoteClient& b)
    {
      return !(a == b);
    }
  }
}
