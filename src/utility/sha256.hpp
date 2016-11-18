// //////////////////////////////////////////////////////////
// sha256.h
// Copyright (c) 2014 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#pragma once

#include <cstdint>
#include <array>
#include <string>

/// compute SHA256 hash
/** Usage:
    sha256::hasher hasher;
    std::string myHash  = sha256("Hello World");     // std::string
    std::string myHash2 = sha256("How are you", 11); // arbitrary data, 11 bytes

    // or in a streaming fashion:

    SHA256 sha256;
    while (more data available)
      sha256.add(pointer to fresh data, number of new bytes);
    std::string myHash3 = sha256.getHash();
  */

namespace hash
{
  class SHA256
  {
  public:
    /// same as reset()
    SHA256();

    using result_type = std::array<std::uint32_t, 8>;

    /// compute SHA256 of a memory block
    result_type operator()(const void* data, size_t numBytes);
    /// compute SHA256 of a string
    result_type operator()(const std::string& text);

    /// add arbitrary number of bytes
    void add(const void* data, size_t numBytes);

    /// return latest hash as 16 hex characters
    result_type operator()();

    /// restart
    void reset();

  private:
    /// process 64 bytes
    void processBlock(const void* data);
    /// process everything left in the internal buffer
    void processBuffer();

    /// split into 64 byte blocks (=> 512 bits)
    enum { BlockSize = 512 / 8, HashValues = 8 };

    /// size of processed data in bytes
    std::uint64_t m_numBytes;
    /// valid bytes in m_buffer
    std::size_t   m_bufferSize;
    /// bytes not processed yet
    std::uint8_t  m_buffer[BlockSize];
    /// hash, stored as integers
    std::uint32_t m_hash[8];
  };
}
