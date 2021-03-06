/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "sound_sample.hpp"

#include <map>

namespace ts
{
  namespace audio
  {
    // The sound sample loader class loads sound samples from files,
    // caching the samples to make sure duplicate requests are reasonably efficient.
    // It comes with a throwing and a non-throwing overload.
    class SoundSampleLoader
    {
    public:
      const SoundSample* load_sound_effect(const std::string& file_name, std::nothrow_t);
      const SoundSample& load_sound_effect(const std::string& file_name);

    private:
      std::map<std::string, SoundSample, std::less<>> lookup_map_;
    };
  }
}