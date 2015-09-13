//
//  KeccakHashImpl.cpp
//  StegoDiskCore
//
//  Created by Martin Kosdy on 14/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#include "KeccakHashImpl.h"
#include "Utils/Keccak/keccak.h"
#include <stdexcept>

namespace StegoDisk {
    
KeccakHashImpl::KeccakHashImpl(std::size_t stateSize)
{
    //TODO: check if stateSize is valid
    this->stateSize = stateSize;
}

KeccakHashImpl::~KeccakHashImpl()
{

}

void KeccakHashImpl::process(MemoryBuffer& state, const uint8* data, std::size_t length)
{
    if (stateSize != state.getSize())
        throw std::length_error("KeccakHashImpl: input state size doesnt match with current state size");
    
    if (data == nullptr)
        throw std::invalid_argument("data pointer cannot be null");
    
    //TOOD: rewrite implementation (int data types)
    //TODO: create new optimized version (sth like FastKeccakImpl / NISTKeccakImpl)
    keccak(data, (int)length, state.getRawPointer(), (int)stateSize);
}

}