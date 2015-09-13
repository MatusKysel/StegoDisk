//
//  FeistelMixPermutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 4/15/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include <algorithm>
#include "Permutations/FeistelMixPermutation.h"
#include "Utils/StegoMath.h"
#include "Utils/Keccak/keccak.h"
#include "Utils/Config.h"
#include "Utils/StegoErrors.h"
#include "Logging/logger.h"
#include "Hash/Hash.h"

#define FMP_MIN_REQ_SIZE                        1024
#define FMP_NUMROUNDS                           5

namespace StegoDisk {

FeistelMixPermutation::FeistelMixPermutation() :
    leftBits(0),
    leftMod(0),
    rightMask(0),
    rightBits(0)
{
	LOG_DEBUG("Permutation::Permutation: called for: " << getNameInstance());
}

FeistelMixPermutation::~FeistelMixPermutation()
{
	LOG_DEBUG("Permutation::~Permutation: destructor called for: " << getNameInstance());
}


void FeistelMixPermutation::init(PermElem requestedSize, Key key)
{
    if (key.getSize() == 0)
        throw std::runtime_error("FeistelMixPermutation init: Invalid key (size=0)");
    
    if (requestedSize < FMP_MIN_REQ_SIZE)
        throw std::runtime_error("FeistelMixPermutation: requestedSize < FMP_MIN_REQ_SIZE (=1024)");
    
    uint8 bitLen = StegoMath::log2(requestedSize);
    
    if (bitLen < 8)
        throw std::runtime_error("FeistelMixPermutation: requested size is too small");
    
    initialized = false;
    
    rightBits = bitLen / 2;
    leftBits = bitLen - rightBits;
    leftMod = (requestedSize >> rightBits);
    rightMask = (((uint64)1) << rightBits)-1;

	// we ensure that rightMask >= leftMod
	if ( rightMask < leftMod ) {
		rightBits++;
		leftBits--;
		leftMod = (requestedSize >> rightBits);
		rightMask = (((uint64)1) << rightBits)-1;
	}

    size = (leftMod) << rightBits;

    // precompute hash table

	// rightMask is always >= leftMod
    uint32 maxHash = (uint32)rightMask;
    
    hashTables = std::vector<std::vector<uint32> >(FMP_NUMROUNDS, std::vector<uint32>(maxHash+1, 0));
    
    //TODO: hash could be initialized by key and then just append "i" in each iteration - or not?
    //      sth like: Hash hash(key.getData());
    //      for (i..) hash.append(..)
    
    Hash hash;
    
	LOG_TRACE("FeistelMixPermutation::init: computing hash table (HT) containing " << maxHash+1 << " elements");
    for (uint32 t=0;t<FMP_NUMROUNDS;t++) {
        for (uint64 i=0; i <= (uint64)maxHash; i++) {
			uint32 index = (uint32) i;
            hash.process(key.getData());
            hash.append((uint8*)&index, sizeof(uint32));
            hash.append((uint8*)&t, sizeof(uint32));
            
            //TODO: rewrite these lines - im sure there is a better way of writing this
            
            if (hash.getStateSize() < 4)
                throw std::runtime_error("hash size is too small");
            
            uint32 hashVal = *((uint32*)hash.getState().getConstRawPointer());
            hashTables[t][index] = hashVal;
        }
    }
	LOG_TRACE("FeistelMixPermutation::init: HT ready; leftMod = " << leftMod << ", rightMask = " << rightMask << endl);
    
    initialized = true;
}

PermElem FeistelMixPermutation::permute(PermElem index) const
{
    commonPermuteInputCheck(index);
    
    uint64 right = index & rightMask;
    uint64 left = index >> rightBits;
    
    // feistel rounds
    for (int r=0; r<FMP_NUMROUNDS; r++) {
        if (r%2) {
            right = (right ^ (hashTables[r][left] & rightMask));
        } else {
            left = (left + hashTables[r][right]) % leftMod;
        }
    }
    
    uint64 permutedIndex = (left << rightBits) | right;

    if (permutedIndex >= size)
        throw std::runtime_error("FeistelMixPermutation: permuted index calculation failed");
    
    return permutedIndex;
}

PermElem FeistelMixPermutation::getSizeUsingParams(PermElem requestedSize, Key key)
{
    if (requestedSize < FMP_MIN_REQ_SIZE) return 0;

    uint8 bitLen = StegoMath::log2(requestedSize);

    if (bitLen < 8)
        return 0;

    uint8 rightBits = bitLen / 2;
    uint64 leftMod = (requestedSize >> rightBits);
    return (leftMod) << rightBits;
}

    
//const string FeistelMixPermutation::getNameInstance() const
//{
//    return getName();
//}
//
//const string FeistelMixPermutation::getName() 
//{
//    return string("MixedFeistel");
//}
//
//shared_ptr<Permutation> FeistelMixPermutation::getNew() 
//{
//    return shared_ptr<Permutation>(new FeistelMixPermutation());
//}
//
//shared_ptr<Permutation> FeistelMixPermutation::getNewInstance() 
//{
//    return getNew();
//}

}
