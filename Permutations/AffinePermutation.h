//
//  AffinePermutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__AffinePermutation__
#define __StegoFS__AffinePermutation__

#include "Permutations/Permutation.h"

namespace StegoDisk {

class AffinePermutation : public Permutation {

public:
    virtual void init(PermElem requestedSize, Key key);
    virtual PermElem permute(PermElem index) const;
    virtual PermElem getSizeUsingParams(PermElem requestedSize, Key key);

    virtual const std::string getNameInstance() const { return "Affine"; };
    
//    const string getNameInstance() const;
//    static const string getName();
//    virtual shared_ptr<Permutation> getNewInstance();
//    static shared_ptr<Permutation> getNew();

protected:
    PermElem getSizeUsingParams(PermElem requestedSize, Key key, bool overwriteMembers);
    uint64 keyParamA;
    uint64 keyParamB;
};
    
}


#endif /* defined(__StegoFS__AffinePermutation__) */
