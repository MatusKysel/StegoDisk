//
//  FeistelNumPermutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 4/15/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include <math.h>       /* sqrt */
#include <algorithm>
#include "Permutations/FeistelNumPermutation.h"
#include "Utils/StegoMath.h"
#include "Utils/Keccak/keccak.h"
#include "Utils/Config.h"
#include "Utils/StegoErrors.h"
#include "Logging/logger.h"
#include "Hash/Hash.h"

#define FNP_MIN_REQ_SIZE                        1024
#define FNP_NUMROUNDS                           5

namespace StegoDisk {

FeistelNumPermutation::FeistelNumPermutation() :
    modulus(0)
{
}

FeistelNumPermutation::~FeistelNumPermutation()
{
}
    

void FeistelNumPermutation::init(PermElem requestedSize, Key key)
{
    if (key.getSize() == 0)
        throw std::runtime_error("FeistelNumPermutation init: Invalid key (size=0)");
    
    if (requestedSize < FNP_MIN_REQ_SIZE)
        throw std::runtime_error("FeistelNumPermutation: requestedSize < FNP_MIN_REQ_SIZE");
    
    initialized = false;
    
    modulus = (uint32) sqrt((double)requestedSize);
    size = (size_t)((uint64)modulus*(uint64)modulus);
    
	// precompute hash table
    uint32 maxHash = modulus;
    
    hashTables = std::vector<std::vector<uint32> >(FNP_NUMROUNDS, std::vector<uint32>(maxHash, 0));
    
    //TODO: hash could be initialized by key and then just append "i" in each iteration - or not?
    //      sth like: Hash hash(key.getData());
    //      for (i..) hash.append(..)
    
    Hash hash;
    
    for (uint32 t=0;t<FNP_NUMROUNDS;t++) {
        for (uint32 i=0; i<maxHash; i++) {
            hash.process(key.getData());
            hash.append((uint8*)&i, 4);
            hash.append((uint8*)&t, 4);
        
            //TODO: rewrite these lines - im sure there is a better way of writing this
            
            if (hash.getStateSize() < 4)
                throw std::runtime_error("hash size is too small");

            uint32 hashVal = *((uint32*)hash.getState().getConstRawPointer());
            hashTables[t][i] = hashVal % modulus;
        }
    }
    
    initialized = true;
}

PermElem FeistelNumPermutation::permute(PermElem index) const
{
    commonPermuteInputCheck(index);
    
    uint64 right = index % modulus;
    uint64 left = index / modulus;
    
    // feistel rounds
    for (std::size_t r=0; r<FNP_NUMROUNDS; r++) {
        if (r%2) {
            right = (right + hashTables[r][left]) % modulus;
        } else {
            left = (left + hashTables[r][right]) % modulus;
        }
    }
    
//    left = (left + hashTables[r++][right]) % modulus;
//    right = (right + hashTables[r++][left]) % modulus;
//    left = (left + hashTables[r++][right]) % modulus;
//    right = (right + hashTables[r++][left]) % modulus;
//    left = (left + hashTables[r++][right]) % modulus;
    
    uint64 permutedIndex = (left * modulus) + right;

    if (permutedIndex >= size)
        throw std::runtime_error("FeistelNumPermutation: permuted index calculation failed");
    
    return permutedIndex;
}

PermElem FeistelNumPermutation::getSizeUsingParams(PermElem requestedSize, Key key)
{
    if (requestedSize < FNP_MIN_REQ_SIZE) return 0;
    uint32 mod = (uint32) sqrt((double)requestedSize);
    return (std::size_t)((uint64)mod*(uint64)mod);
}


//const string FeistelNumPermutation::getNameInstance() const
//{
//    return getName();
//}
//
//const string FeistelNumPermutation::getName() 
//{
//    return string("NumericFeistel");
//}
//
//shared_ptr<Permutation> FeistelNumPermutation::getNew() 
//{
//    return shared_ptr<Permutation>(new FeistelNumPermutation());
//}
//
//shared_ptr<Permutation> FeistelNumPermutation::getNewInstance() 
//{
//    return getNew();
//}

}
