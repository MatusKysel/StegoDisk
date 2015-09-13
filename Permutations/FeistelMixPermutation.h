//
//  FeistelMixPermutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 4/15/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__FeistelMixPermutation__
#define __StegoFS__FeistelMixPermutation__

#include "Permutations/Permutation.h"
#include <vector>

namespace StegoDisk {

class FeistelMixPermutation : public Permutation {
    
public:
	FeistelMixPermutation();
    ~FeistelMixPermutation();
    
    virtual void init(PermElem requestedSize, Key key);
    virtual PermElem permute(PermElem index) const;
    virtual PermElem getSizeUsingParams(PermElem requestedSize, Key key);

    virtual const std::string getNameInstance() const { return "MixedFeistel"; };
    
//    const string getNameInstance() const;
//    static const string getName();
//    virtual shared_ptr<Permutation> getNewInstance();
//    static shared_ptr<Permutation> getNew();
    
private:

    std::vector< std::vector<uint32> > hashTables;

    uint8 leftBits;
    uint64 leftMod;

    uint64 rightMask;
    uint8 rightBits;
};

}

#endif /* defined(__StegoFS__FeistelMixPermutation__) */
