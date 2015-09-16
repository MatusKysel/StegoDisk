//
//  HashImpl.cpp
//  StegoDiskCore
//
//  Created by Martin Kosdy on 14/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#include "hash_impl.h"

namespace stego_disk {

void HashImpl::Append(MemoryBuffer& state,
                      const uint8* data, std::size_t length) {
  if (state_size_ != state.GetSize())
    throw std::length_error("HashImpl: input state size doesnt "
                            "match with current state size");

  //  METHOD1: newState = oldState ^ hash(input)
  //    MemoryBuffer buffer(stateSize);
  //    this->process(buffer, data, length);
  //    state ^= buffer;

  //  METHOD2: newState = hash(hash(input) | oldState) - where | is concat
  MemoryBuffer hashed_input(state_size_);
  this->Process(hashed_input, data, length); //TODO(Matus) podla mna staci aj bez this

  MemoryBuffer buffer(state_size_ * 2);
  // 1st half - hash(input)
  buffer.Write(0, hashed_input.GetRawPointer(), hashed_input.GetSize());
  // 2nd half - oldState
  buffer.Write(state_size_, state.GetRawPointer(), state_size_);

  this->Process(state, buffer.GetRawPointer(), buffer.GetSize()); //TODO(Matus) podla mna staci aj bez this
}

} // stego_disk
