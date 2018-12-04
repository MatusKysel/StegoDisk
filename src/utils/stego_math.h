/**
* @file stego_math.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Class with all mathematical operation
*
*/

#pragma once

#include <string>

#include "api_mask.h"
#include "stego_header.h"
#include "memory_buffer.h"

namespace stego_disk {

class StegoMath {

public:
  static uint64 Gcd(uint64 a, uint64 b);
  static uint64 Lcm(uint64 a, uint64 b);
  static bool MillerRabin(uint64 p);
  static void PrintHexBuffer(const uint8 *buffer, std::size_t length);
  static std::string HexBufferToStr(const uint8 *buffer, std::size_t length);
  static std::string HexBufferToStr(const MemoryBuffer& buffer);
  static uint64 ClosestSmallerPrime(uint64 number);
  static uint64 Modulo(uint64 a, uint64 b, uint64 c);
  static uint64 Mulmod(uint64 a, uint64 b, uint64 m);
  static uint8 Log2(uint64 number);
  static uint8 Popcount(uint64 x);

};

} // stego_disk
