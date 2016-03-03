/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef DEBUG_LOG_11232_HPP
#define DEBUG_LOG_11232_HPP

#include "logger.hpp"

#include <cstdint>

namespace ts
{
  namespace debug
  {
    struct DebugTag {};

    using logger::flush;
    using logger::endl;

    namespace level
    {
      static const std::uint32_t essential = 0;
      static const std::uint32_t relevant = 1;
      static const std::uint32_t auxiliary = 2;
    }

#define DEBUG_ESSENTIAL debug::Log(debug::level::essential)
#define DEBUG_RELEVANT debug::Log(debug::level::relevant)
#define DEBUG_AUXILIARY debug::Log(debug::level::auxiliary)

    struct DebugConfig
    {
      std::uint32_t debug_level = level::essential;
    };
  }

  namespace logger
  {
    template <>
    struct LoggerTraits<debug::DebugTag>
    {
      using config_type = debug::DebugConfig;
      using dispatcher_type = LogFileDispatcher;
    };
  }
  
  namespace debug
  {
    struct Log
    {
    public:
      Log(std::uint32_t debug_level);

      template <typename T>
      Log& operator<<(const T& value);

    private:
      logger::LoggerFront<DebugTag> logger_;
      std::uint32_t debug_level_;
      std::uint32_t debug_config_level_;
    };

    using ScopedLogger = logger::ScopedLogger<DebugTag>;

    inline Log::Log(std::uint32_t debug_level)
      : logger_(),
      debug_level_(debug_level),
      debug_config_level_(logger_.config().debug_level)
    {
    }

    template <typename T>
    Log& Log::operator<<(const T& value)
    {
      if (debug_config_level_ >= debug_level_)
      {
        logger_ << value;
      }

      return *this;
    }
  }
}

#endif