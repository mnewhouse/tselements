/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "config.hpp"

// This header must be included in *one* source file, so that the
// configuration definitions are linked with the binary.
// Before including this file, you can define any of the macros
// that affect the configuration values.


#ifndef TS_DATA_DIRECTORY
#define TS_DATA_DIRECTORY "data"
#endif

#ifndef TS_AUDIO_DIRECTORY
#define TS_AUDIO_DIRECTORY "sound"
#endif

// The data directory is where the game looks for default track assets.
const char* const ts::config::data_directory = TS_DATA_DIRECTORY;
const char* const ts::config::audio_directory = TS_AUDIO_DIRECTORY;
