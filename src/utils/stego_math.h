//
//  StegoMath.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/23/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_UTILS_STEGOMATH_H_
#define STEGODISK_UTILS_STEGOMATH_H_

#include <string>

#include "stego_header.h"
#include "memory_buffer.h"

using namespace std;

namespace stego_disk {

class StegoMath {

public:
  static uint64 Gcd(uint64 a, uint64 b);
  static uint64 Lcm(uint64 a, uint64 b);
  //static bool MillerRabin(_uint64 p, int iteration);
  static bool MillerRabin(uint64 p);
  static void PrintHexBuffer(const uint8 *buffer, std::size_t length);
  static string HexBufferToStr(const uint8 *buffer, std::size_t length);
  static string HexBufferToStr(const MemoryBuffer& buffer);
  static uint64 ClosestSmallerPrime(uint64 number);
  static uint64 Modulo(uint64 a, uint64 b, uint64 c);
  //static long long mulmod(long long a,long long b,long long c);
  //static inline _uint64_t mulmod(_uint64_t a, _uint64_t b, _uint64_t m);
  static uint64 Mulmod(uint64 a, uint64 b, uint64 m);
  static uint8 Log2(uint64 number);
  static uint8 Popcount(uint64 x);

};

} // stego_disk

#endif // STEGODISK_UTILS_STEGOMATH_H_
