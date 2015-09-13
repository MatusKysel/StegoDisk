//
//  FeistelNetPermutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 4/15/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__FeistelNumPermutation__
#define __StegoFS__FeistelNumPermutation__

#include "Permutations/Permutation.h"
#include "Utils/StegoTypes.h"
#include <vector>

namespace StegoDisk {
    
class FeistelNumPermutation : public Permutation {
    
public:
    FeistelNumPermutation();
    ~FeistelNumPermutation();
    
    virtual void init(PermElem requestedSize, Key key);
    virtual PermElem permute(PermElem index) const;
    virtual PermElem getSizeUsingParams(PermElem requestedSize, Key key);
    
    virtual const std::string getNameInstance() const { return "NumericFeistel"; };
    
//    const string getNameInstance() const;
//    static const string getName();
//    virtual shared_ptr<Permutation> getNewInstance();
//    static shared_ptr<Permutation> getNew();

private:
    std::vector< std::vector<uint32> > hashTables;

    uint32 modulus;
};
    
}

#endif /* defined(__StegoFS__FeistelNumPermutation__) */
