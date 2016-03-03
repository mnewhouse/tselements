/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SOUND_SAMPLE_HPP_58198192835
#define SOUND_SAMPLE_HPP_58198192835

#include <string>

#include <SFML/Audio/SoundBuffer.hpp>

namespace ts
{
  namespace audio
  {
    using SoundSample = sf::SoundBuffer;

    SoundSample load_sound_from_file(const std::string& file_name);
  }
}

#endif