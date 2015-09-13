//
//  StegoMath.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/23/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__StegoMath__
#define __StegoFS__StegoMath__

#include <string>
#include "Utils/StegoHeader.h"
#include "Utils/MemoryBuffer.h"

using namespace std;

namespace StegoDisk {

class StegoMath {

public:
    static uint64 gcd(uint64 a, uint64 b);
    static uint64 lcm(uint64 a, uint64 b);
    //static bool MillerRabin(_uint64 p, int iteration);
    static bool MillerRabin(uint64 p);
    static void printHexBuffer(const uint8 *buffer, std::size_t length);
    static string hexBufferToStr(const uint8 *buffer, std::size_t length);
    static string hexBufferToStr(const MemoryBuffer& buffer);
    static uint64 closestSmallerPrime(uint64 number);
    static uint64 modulo(uint64 a, uint64 b, uint64 c);
    //static long long mulmod(long long a,long long b,long long c);
    //static inline _uint64_t mulmod(_uint64_t a, _uint64_t b, _uint64_t m);
    static uint64 mulmod(uint64 a, uint64 b, uint64 m);
    static uint8 log2(uint64 number);
    static uint8 popcount(uint64 x);
    
};

}

#endif /* defined(__StegoFS__StegoMath__) */
