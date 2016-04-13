/**
* @file stego_math.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Class with implementation of all mathematical operation
*
*/

#include "stego_math.h"

#include <stdlib.h>
#include <stdio.h>

#include <sstream>
#include <iomanip>

namespace stego_disk {

uint64 StegoMath::Gcd(uint64 a, uint64 b) {
  while ( 1 ) {
    a = a % b;
    if( a == 0 )
      return b;
    b = b % a;

    if( b == 0 )
      return a;
  }
}

uint64 StegoMath::Lcm(uint64 a, uint64 b) {
  return a * b / Gcd(a,b);
}

/*
 *  Miller-Rabin Primality Test
 *
 * source: http://community.topcoder.com/tc?module=Static&d1=tutorials&d2=primalityTesting
 */
bool StegoMath::MillerRabin(uint64 p) {
  if(p < 2) {
    return false;
  }
  if(p != 2 && p %2 == 0) {
    return false;
  }
  uint64 s = p - 1;
  while (s % 2 == 0) {
    s /= 2;
  }

  /*
    if n < 1,373,653, it is enough to test a = 2 and 3;
    if n < 9,080,191, it is enough to test a = 31 and 73;
    if n < 4,759,123,141, it is enough to test a = 2, 7, and 61;
    if n < 1,122,004,669,633, it is enough to test a = 2, 13, 23, and 1662803;
    if n < 2,152,302,898,747, it is enough to test a = 2, 3, 5, 7, and 11;
    if n < 3,474,749,660,383, it is enough to test a = 2, 3, 5, 7, 11, and 13;
    if n < 341,550,071,728,321, it is enough to test a = 2, 3, 5, 7, 11, 13, and 17.
    */

  uint64 witness[] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,
                      73,79,83,89,97,1662803};
  //if (iterations > (sizeof(witness) >> 3)) iterations = it;
  int iterations = (sizeof(witness) >> 3);


  for (int i = 0; i < iterations; ++i) {
    uint64 a = rand() % (p - 1) + 1, temp = s;
    uint64 mod = Modulo(a, temp, p);
    while (temp != p - 1 && mod != 1 && mod != p - 1) {
      mod = Mulmod(mod, mod, p);
      temp *= 2;
    }
    if(mod != p - 1 && temp % 2 == 0) {
      return false;
    }
  }
  return true;
}


/* This function calculates (a^b)%c */
uint64 StegoMath::Modulo(uint64 a, uint64 b, uint64 c) {

#if __GNUC__
#if __x86_64__ || __ppc64__
  __unit128_t x = 1, y = a;
#else
  uint64 x = 1, y = a;
#endif
#endif

  while (b > 0) {
    if( b % 2 == 1) {
      x=(x * y) % c;
    }
    y = (y * y) % c; // squaring the base
    b /= 2;
  }
  return x % c;
}

uint64 StegoMath::Mulmod(uint64 a,uint64 b,uint64 m) {
  if ((a < 0xFFFFFF) && (b < 0xFFFFFF)) {
    return (a * b) % m;
  }

  uint64 res = 0;
  while (a != 0) {
    if (a & 1) res = (res + b) % m;
    a >>= 1;
    b = (b << 1) % m;
  }
  return res;
}

/*
 * Finds the closest smaller prime to the number
 *
 * @return closest smaller prime or 0 if there is no smaller prime than number
 */
uint64 StegoMath::ClosestSmallerPrime(uint64 number) {
  if (number < 2) return 0;
  if (number % 2 == 0) number--;

  srand(2);

  while (!MillerRabin(number) && (number > 2)) {
    number -= 2;
  }

  if (number > 1) {
    return number;
  } else {
    return 0;
  }

}

void StegoMath::PrintHexBuffer(const uint8 *buffer, std::size_t length) {
  for (std::size_t i = 0; i < length; ++i) {
    printf("%02x ", (unsigned char)buffer[i]);
  }
  printf("\n");
}


string StegoMath::HexBufferToStr(const uint8 *buffer, std::size_t length) {
  std::stringstream buf;
  buf << std::hex;

  if (buffer == nullptr)
    return "";

  if (length == 0)
    return "";


  for (std::size_t i = 0; i < length; ++i) {
    buf << static_cast<int>(buffer[i]) << " ";
  }

  return buf.str();
}


string StegoMath::HexBufferToStr(const MemoryBuffer& buffer) {
  return HexBufferToStr(buffer.GetConstRawPointer(), buffer.GetSize());
}

// TODO: changed output from uint32 to uint8 ??? is correct?
uint8 StegoMath::Log2(uint64 number) {
  uint8 leadingZeros = 0;

  if (!number)
    return 0;

  while (((number >> (63 - leadingZeros)) & 1) == 0)
    leadingZeros++;

  return 64 - leadingZeros;
}


uint8 StegoMath::Popcount(uint64 x) {
  x -= (x >> 1) & 0x5555555555555555;
  x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
  x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
  x += x >> 8;
  x += x >> 16;
  x += x >> 32;
  return x & 0x7f;
}

} // stego_disk
