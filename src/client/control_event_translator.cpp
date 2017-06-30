/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "control_event_translator.hpp"
#include "local_message_dispatcher.hpp"

#include "stage/stage_messages.hpp"
#include "stage/stage.hpp"

#include "controls/key_mapping.hpp"
#include "controls/control_center.hpp"

namespace ts
{
  namespace client
  {
    ControlEventTranslator::ControlEventTranslator(const stage::Stage* stage,
                                                   controls::ControlCenter* control_center,
                                                   KeyMapping key_mapping)
      : stage_(stage),
        control_center_(control_center),
        key_mapping_(std::move(key_mapping))
    {}
  }
}