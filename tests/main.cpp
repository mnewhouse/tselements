/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "utility/debug_log.hpp"

#include <locale>
#include <codecvt>

using namespace ts;

int main(int argc, char* argv[])
{
  // global setup...

  std::locale::global({ {}, new std::codecvt_utf8<wchar_t> });

  debug::DebugConfig debug_config;
  debug_config.debug_level = debug::level::auxiliary;
  debug::ScopedLogger debug_log(debug_config, "debug.txt");  

  int result = Catch::Session().run(argc, argv);

  return result;
}


#define TS_DATA_DIRECTORY "assets/data"

#include "core/config_definitions.hpp"