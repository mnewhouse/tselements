/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "subsystem_initialization.hpp"

#include <stdexcept>

namespace ts
{
  namespace core
  {
    static std::runtime_error initialization_error(const char* subsystem, const char* specific_error)
    {
      std::string message = "Failed to initialize subsystem '";
      message += subsystem;
      message += "'";
      message += specific_error;

      return std::runtime_error(message);
    }

    void initialize_subsystems()
    {
    }
  }
}