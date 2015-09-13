//
//  Affine64Permutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include "Permutations/Affine64Permutation.h"
#include "Utils/StegoMath.h"
#include "Utils/StegoErrors.h"
#include "Utils/Config.h"
#include "Logging/logger.h"

namespace StegoDisk {

void Affine64Permutation::init(PermElem requestedSize, Key key)
{
    initialized = false;
    
    if (getSizeUsingParams(requestedSize, key, true) == 0)
        throw std::runtime_error("Affine64Permutation: requested size is too small");
    
    initialized = true;
}

PermElem Affine64Permutation::permute(PermElem index) const
{
    commonPermuteInputCheck(index);
    
    return (StegoMath::mulmod(index, keyParamA, size) + keyParamB) % size;
}


    
//const string Affine64Permutation::getNameInstance() const
//{
//    return getName();
//}
//
//const string Affine64Permutation::getName() 
//{
//    return string("Affine64");
//}
//
//shared_ptr<Permutation> Affine64Permutation::getNew() 
//{
//    return shared_ptr<Permutation>(new Affine64Permutation());
//}
//
//shared_ptr<Permutation> Affine64Permutation::getNewInstance() 
//{
//    return getNew();
//}
    
}


