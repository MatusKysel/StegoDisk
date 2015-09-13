//
//  AffinePermutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include "Permutations/AffinePermutation.h"
#include "Utils/StegoMath.h"
#include "Utils/StegoErrors.h"
#include "Utils/Config.h"
#include "Logging/logger.h"

namespace StegoDisk {
    
PermElem AffinePermutation::getSizeUsingParams(PermElem requestedSize, Key key, bool overwriteMembers)
{
    if (key.getSize() == 0)
        throw std::runtime_error("AffinePermutation: Invalid key (size=0)");

    uint64 prime = requestedSize;
    
    uint64 a, b;

    while(1) {

        prime = StegoMath::closestSmallerPrime(prime);
        if (prime < 1) {
            return 0;
        }

        a = 0;
        b = 0;

        // TODO: modify this to handle different key lengths
        if (key.getSize() < 8)
            throw std::invalid_argument("AffinePermutation: key must be longer than 8 bytes");
        
        if ((key.getSize() % 16) != 0)
            throw std::invalid_argument("AffinePermutation: wrong key length! should be nonzero multiple of 16");
        
        for (std::size_t i=0; i<key.getSize()/2; i+=8) {
            a ^= (uint64)*(uint64*)(&key.getData()[i]);
        }
        for (std::size_t i=key.getSize()/2;i<key.getSize();i+=8) {
            b ^= (uint64)*(uint64*)(&key.getData()[i]);
        }

        a %= (prime / 2);
        b %= (prime / 2);

        a += (prime/2);
        b += (prime/2);

        if (StegoMath::gcd(a, prime) == 1) {
            if (overwriteMembers) {
                size = prime;
                keyParamA = a;
                keyParamB = b;
            }
            return prime;
        }
    }
}

PermElem AffinePermutation::getSizeUsingParams(PermElem requestedSize, Key key)
{
    if (requestedSize > 0xFFFFFFFF) // max size = 2^32 blocks (due to slow mulmod aritmethics for numbers > 2^32)
        return 0;

    return getSizeUsingParams(requestedSize, key, false);
}


void AffinePermutation::init(PermElem requestedSize, Key key)
{
    if (requestedSize > 0xFFFFFFFF) // max size = 2^32 blocks (due to slow mulmod aritmethics for numbers > 2^32)
        throw std::out_of_range("AffinePermutation: max requested size is (2^32)-1");

    initialized = false;
    
    if (getSizeUsingParams(requestedSize, key, true) == 0)
        throw std::runtime_error("AffinePermutation: requested size is too small");

    initialized = true;
    
    //LOG_DEBUG("AffinePermutation::init: hash is " << StegoMath::hexBufferToStr(keyHash, SFS_KEY_HASH_LENGTH) << ", size: " << _size << ", paramA: " << _keyParamA << ", paramB: " << _keyParamB);
}

    
PermElem AffinePermutation::permute(PermElem index) const
{
    commonPermuteInputCheck(index);
    return ((((index)*keyParamA) % size) + keyParamB) % size;
}
    
//
//const string AffinePermutation::getNameInstance() const
//{
//    return getName();
//}
//
//const string AffinePermutation::getName() 
//{
//    return string("Affine");
//}
//
//shared_ptr<Permutation> AffinePermutation::getNew() 
//{
//    return shared_ptr<Permutation>(new AffinePermutation());
//}
//
//shared_ptr<Permutation> AffinePermutation::getNewInstance() 
//{
//    return getNew();
//}

}
