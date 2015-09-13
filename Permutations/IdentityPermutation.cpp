//
//  IdentityPermutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include "Permutations/IdentityPermutation.h"
#include "Utils/StegoHeader.h"

namespace StegoDisk {

IdentityPermutation::IdentityPermutation()
{
}

IdentityPermutation::~IdentityPermutation()
{
}

void IdentityPermutation::init(PermElem requestedSize, Key key)
{
//    if (key.getSize() == 0)
//        throw std::runtime_error("IdentityPermutatuion init: Invalid key (size=0)");

    if (size == 0)
        throw std::invalid_argument("IdentityPermutation: requestedSize cannot be 0");
    
    size = requestedSize;
}

PermElem IdentityPermutation::permute(PermElem index) const
{
    commonPermuteInputCheck(index);

    return index;
}

PermElem IdentityPermutation::getSizeUsingParams(PermElem requestedSize, Key key)
{
    return requestedSize;
}


//const string IdentityPermutation::getNameInstance() const
//{
//    return getName();
//}
//
//const string IdentityPermutation::getName() 
//{
//    return string("Identity");
//}
//
//shared_ptr<Permutation> IdentityPermutation::getNew() 
//{
//    return shared_ptr<Permutation>(new IdentityPermutation());
//}
//
//shared_ptr<Permutation> IdentityPermutation::getNewInstance() 
//{
//    return getNew();
//}


}