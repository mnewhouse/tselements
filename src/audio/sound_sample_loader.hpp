/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SOUND_SAMPLE_LOADER_HPP_4149012905
#define SOUND_SAMPLE_LOADER_HPP_4149012905

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

#endif