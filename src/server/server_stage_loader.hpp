/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SERVER_STAGE_LOADER_HPP_8198123894
#define SERVER_STAGE_LOADER_HPP_8198123894

#include "stage/stage_loader.hpp"

namespace ts
{
  namespace cup
  {
    namespace messages
    {
      struct PreInitialization;
    }
  }

  namespace server
  {
    class MessageConveyor;

    class StageLoader
      : private stage::StageLoader
    {
    public:
      void handle_message(cup::messages::PreInitialization&& pre_initialization);

      using stage::StageLoader::get_result;
      using stage::StageLoader::is_loading;
      using stage::StageLoader::is_ready;
      using stage::StageLoader::loading_state;
      using stage::StageLoader::progress;
      using stage::StageLoader::max_progress;    
    };
  }
}

#endif