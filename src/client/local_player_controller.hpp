/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOCAL_PLAYER_CONTROLLER_HPP_5590191823
#define LOCAL_PLAYER_CONTROLLER_HPP_5590191823

#include <boost/container/small_vector.hpp>

#include "cup/player_definition.hpp"

namespace ts
{
  namespace controls
  {
    class ControlCenter;
  }

  namespace stage
  {
    struct StageDescription;
  }

  namespace cup
  {
    namespace messages
    {
      struct RegistrationSuccess;
    }
  }

  namespace client
  {
    class LocalPlayerController
    {
    public:
      explicit LocalPlayerController(const cup::PlayerDefinition* players, std::size_t player_count);

      controls::ControlCenter create_control_center(const stage::StageDescription& stage_desc) const;

      void handle_message(const cup::messages::RegistrationSuccess& success);

    private:
      boost::container::small_vector<cup::PlayerDefinition, 8> local_players_;
      std::uint16_t client_id_ = 0;
    };
  }
}

#endif