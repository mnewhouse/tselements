/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef GUI_SOUND_STYLE_HPP_82591825
#define GUI_SOUND_STYLE_HPP_82591825

#include "audio/sound_sample.hpp"

namespace ts
{
  namespace gui
  {
    namespace styles
    {
      struct ClickEventSound
      {
        const audio::SoundSample& sound;
      };

      auto click_playback_sound(const audio::SoundSample& sound_sample)
      {
        return ClickEventSound{ sound_sample };
      }
    }
  }
}

#endif
