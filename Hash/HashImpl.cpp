//
//  HashImpl.cpp
//  StegoDiskCore
//
//  Created by Martin Kosdy on 14/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#include "HashImpl.h"

namespace StegoDisk {

void HashImpl::append(MemoryBuffer& state, const uint8* data, std::size_t length)
{
    if (stateSize != state.getSize())
        throw std::length_error("HashImpl: input state size doesnt match with current state size");

//  METHOD1: newState = oldState ^ hash(input)
//    MemoryBuffer buffer(stateSize);
//    this->process(buffer, data, length);
//    state ^= buffer;

//  METHOD2: newState = hash(hash(input) | oldState) - where | is concat
    MemoryBuffer hashedInput(stateSize);
    this->process(hashedInput, data, length);
    
    MemoryBuffer buffer(stateSize*2);
    // 1st half - hash(input)
    buffer.write(0, hashedInput.getRawPointer(), hashedInput.getSize());
    // 2nd half - oldState
    buffer.write(stateSize, state.getRawPointer(), stateSize);
    
    this->process(state, buffer.getRawPointer(), buffer.getSize());
}
    
}
