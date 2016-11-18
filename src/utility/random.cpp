/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "random.hpp"

#include <chrono>

namespace ts
{
  namespace utility
  {
    namespace detail
    {
      static std::mt19937_64 initialize_engine()
      {
        std::random_device device;
        std::uint64_t epoch_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        std::initializer_list<std::uint64_t> seed = 
        {
          epoch_time << 32, epoch_time, device(), device(), device(), device(), device(), device()
        };
        
        return std::mt19937_64(std::seed_seq(seed));
      }

      std::mt19937_64& random_engine()
      {
        static thread_local std::mt19937_64 engine = initialize_engine();
        return engine;
      }
    }
  }
}