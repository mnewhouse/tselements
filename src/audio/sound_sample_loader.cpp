/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "sound_sample_loader.hpp"

#include "utility/stream_utilities.hpp"

namespace ts
{
  namespace audio
  {
    const SoundSample& SoundSampleLoader::load_sound_effect(const std::string& file_name)
    {
      auto it = lookup_map_.find(file_name);
      if (it != lookup_map_.end())
      {
        return it->second;
      }
      
      auto result = lookup_map_.insert(std::make_pair(file_name, load_sound_from_file(file_name)));
      return result.first->second;
    }
  }
}