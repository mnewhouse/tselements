/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef LOCAL_PLAYER_ROSTER_HPP_5590191823
#define LOCAL_PLAYER_ROSTER_HPP_5590191823

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
    // The local player roster keeps track of the locally controlled players,
    // and can create a control center based on which entities' controller id
    // match our local client id.
    class LocalPlayerRoster
    {
    public:
      explicit LocalPlayerRoster(const cup::PlayerDefinition* players, std::size_t player_count);

      controls::ControlCenter create_control_center(const stage::StageDescription& stage_desc) const;

      // This function must be called upon registration, to make sure we have the correct client id.
      void registration_success(std::uint16_t client_id);

      const cup::PlayerDefinition* players() const;
      std::size_t player_count() const;

    private:
      boost::container::small_vector<cup::PlayerDefinition, 8> local_players_;
      std::uint16_t client_id_ = 0;
    };
  }
}

#endif