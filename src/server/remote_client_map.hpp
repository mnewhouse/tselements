/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "remote_client.hpp"

#include <boost/optional.hpp>

#include <array>
#include <cstdint>
#include <vector>

namespace ts
{
  namespace server
  {
    class RemoteClientMap
    {
    public:
      explicit RemoteClientMap(std::uint16_t max_client_count);

      RemoteClient& operator[](std::uint16_t client_id);
      const RemoteClient& operator[](std::uint16_t client_id) const;

      const RemoteClient* find(std::uint16_t client_id) const;
      RemoteClient* find(std::uint16_t client_id);

      boost::optional<std::uint16_t> find(const RemoteClient& client) const;

      void insert(uint16_t client_id, RemoteClient remote_client);
      void erase(uint16_t client_id);

    private:
      std::vector<boost::optional<RemoteClient>> client_map_;
      std::uint16_t map_end_ = 0;
    };
  }
}
