/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef MESSAGE_VIEW_HPP_19835018925
#define MESSAGE_VIEW_HPP_19835018925

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

#endif
