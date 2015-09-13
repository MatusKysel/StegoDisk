//
//  PrimeFieldPermutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__Affine64Permutation__
#define __StegoFS__Affine64Permutation__

#include "Permutations/AffinePermutation.h"

namespace StegoDisk {

class Affine64Permutation : public AffinePermutation {

public:
    virtual void init(PermElem requestedSize, Key key);
    virtual PermElem permute(PermElem index) const;
    //virtual PermElem getSizeUsingParams(PermElem requestedSize, Key key);

    virtual const std::string getNameInstance() const { return "Affine64"; };
    
//    const string getNameInstance() const;
//    static const string getName();
//    virtual shared_ptr<Permutation> getNewInstance();
//    static shared_ptr<Permutation> getNew();
};

}

#endif /* defined(__StegoFS__Affine64Permutation__) */
