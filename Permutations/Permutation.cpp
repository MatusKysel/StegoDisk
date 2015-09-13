//
//  Permutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include <string.h> // memset

#include "Utils/StegoHeader.h"
#include "Permutations/Permutation.h"

namespace StegoDisk {

Permutation::Permutation() : size(0), initialized(false)
{
}

Permutation::~Permutation()
{
}

PermElem Permutation::getSize() const
{
    return size;
}

void Permutation::commonPermuteInputCheck(PermElem index) const
{
    if (index >= size)
        throw std::out_of_range("Permutation: element index out of range");
    
    if (!initialized)
        throw std::runtime_error("Permutation: permutation must be initialized before use");
}
    
}