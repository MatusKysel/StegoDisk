/**
* @file hash_impl.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Interface for hash implementations
*
*/

#include "api_mask.h"
#include "hash_impl.h"

namespace stego_disk {

void HashImpl::Append(MemoryBuffer& state,
                      const uint8* data, std::size_t length) {
  if (state_size_ != state.GetSize())
    throw std::length_error("HashImpl: input state size doesnt "
                            "match with current state size");

  MemoryBuffer hashed_input(state_size_);
  this->Process(hashed_input, data, length);

  MemoryBuffer buffer(state_size_ * 2);
  // 1st half - hash(input)
  buffer.Write(0, hashed_input.GetRawPointer(), hashed_input.GetSize());
  // 2nd half - oldState
  buffer.Write(state_size_, state.GetRawPointer(), state_size_);

  this->Process(state, buffer.GetRawPointer(), buffer.GetSize());
}

} // stego_disk
