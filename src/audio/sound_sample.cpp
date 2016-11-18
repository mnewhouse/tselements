/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "sound_sample.hpp"

#include "utility/stream_utilities.hpp"

#include <SFML/Audio.hpp>

#include <string>
#include <stdexcept>

namespace ts
{
  namespace audio
  {
    static std::runtime_error audio_load_error(const std::string& file_name, const char* detailed_error)
    {
      std::string error_msg = "Failed to load sound file '";
      error_msg += file_name;
      error_msg += "': ";
      error_msg += detailed_error;
      return std::runtime_error(error_msg);
    }

    SoundSample load_sound_from_file(const std::string& file_name)
    {
      auto stream = make_ifstream(file_name, std::ios::binary | std::ios::in);
      if (!stream)
      {
        throw audio_load_error(file_name, "could not open file for reading");
      }

      SoundSample sample;
      auto contents = read_stream_contents(stream);
      if (!sample.loadFromMemory(contents.data(), contents.size()))
      {
        throw audio_load_error(file_name, "invalid file format");
      }

      return sample;
    }    
  }
}