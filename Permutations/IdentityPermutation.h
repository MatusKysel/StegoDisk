//
//  IdentityPermutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__IdentityPermutation__
#define __StegoFS__IdentityPermutation__

#include "Permutations/Permutation.h"

namespace StegoDisk {

class IdentityPermutation : public Permutation {

public:
    IdentityPermutation();
    ~IdentityPermutation();
    
    virtual void init(PermElem requestedSize, Key key);
    virtual PermElem permute(PermElem index) const;
    virtual PermElem getSizeUsingParams(PermElem requestedSize, Key key);
    
    virtual const std::string getNameInstance() const { return "Identity"; };
    
//    const string getNameInstance() const;
//    static const string getName();
//    virtual shared_ptr<Permutation> getNewInstance();
//    static shared_ptr<Permutation> getNew();

};
    
}


#endif /* defined(__StegoFS__IdentityPermutation__) */
