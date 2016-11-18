/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <cstdint>
#include <cstddef>

#include <boost/range/iterator_range.hpp>

namespace ts
{
  namespace messages
  {
    using MessageView = boost::iterator_range<const std::uint8_t*>;
  }
}
