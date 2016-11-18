/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"

#include "remote_client_map.hpp"

#include <algorithm>

namespace ts
{
  namespace server
  {
    RemoteClientMap::RemoteClientMap(std::uint16_t max_client_count)
      : client_map_(max_client_count, boost::none)
    {
    }

    const RemoteClient& RemoteClientMap::operator[](std::uint16_t client_id) const
    {
      return *client_map_[client_id];
    }

    RemoteClient& RemoteClientMap::operator[](std::uint16_t client_id)
    {
      return *client_map_[client_id];
    }

    const RemoteClient* RemoteClientMap::find(std::uint16_t client_id) const
    {
      return client_map_[client_id].get_ptr();
    }

    RemoteClient* RemoteClientMap::find(std::uint16_t client_id)
    {
      return client_map_[client_id].get_ptr();
    }

    boost::optional<std::uint16_t> RemoteClientMap::find(const RemoteClient& client) const
    {
      auto begin = client_map_.begin(), end = begin + map_end_;
      auto it = std::find_if(begin, end, 
                             [client](const auto& entry)
      {
        return entry && client == *entry;
      });

      if (it != end)
      {
        return static_cast<std::uint16_t>(std::distance(begin, it));
      }

      return boost::none;
    }

    void RemoteClientMap::insert(std::uint16_t client_id, RemoteClient remote_client)
    {
      if (!client_map_[client_id])
      {
        client_map_[client_id].emplace(std::move(remote_client));

        if (client_id >= map_end_) map_end_ = client_id + 1;
      }
    }

    void RemoteClientMap::erase(std::uint16_t client_id)
    {
      client_map_[client_id] = boost::none;

      if (map_end_ - 1 == client_id)
      {
        while (map_end_ != 0 && client_map_[client_id--] == boost::none)
        {
          --map_end_;
        }
      }
    }
  }
}