//
//  Permutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_PERMUTATION_CONFIG_H___StegoFS__Permutation__
#define __StegoFS__Permutation__

#include <string>
#include <memory>
#include "Utils/StegoTypes.h"
#include "Keys/Key.h"

namespace StegoDisk {

typedef uint64 PermutationElement;
typedef PermutationElement PermElem;
typedef PermutationElement PermSize;

class Permutation {

public:
    Permutation();
    virtual ~Permutation();

    // abstract methods
    virtual void init(PermElem requestedSize, Key key) = 0;
    virtual PermElem permute(PermElem index) const = 0;
    virtual PermElem getSizeUsingParams(PermElem requestedSize, Key key) = 0;

    virtual const std::string getNameInstance() const = 0;

    //TODO: not sure if this is a good practice
//    static const string getName();
//    virtual shared_ptr<Permutation> getNewInstance() = 0;

    // element access using [] operator
    PermElem& operator[](PermElem index);
    const PermElem& operator[](PermElem index) const;

    PermElem getSize() const;

    bool isInitialized() const { return initialized; };

protected:
    void commonPermuteInputCheck(PermElem index) const; // throws exceptions (out_of_range, runtime - not initialized)

    PermElem size;
    bool initialized;
};

}

#endif /* defined(__StegoFS__Permutation__) */
