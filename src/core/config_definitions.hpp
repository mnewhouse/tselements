/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CONFIG_DEFINITIONS_HPP_358938
#define CONFIG_DEFINITIONS_HPP_358938

#include "config.hpp"

// This header must be included in *one* source file, so that the
// configuration definitions are linked with the binary.
// Before including this file, you can define any of the macros
// that affect the configuration values.


#ifndef TS_DATA_DIRECTORY
#define TS_DATA_DIRECTORY "data"
#endif

// The data directory is where the game looks for default track assets.
const char* const ts::config::data_directory = TS_DATA_DIRECTORY;

#endif