/**
* @file keccak_hash_impl.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Implementation of hash function Keccak
*
*/

#include "keccak_hash_impl.h"

#include <stdexcept>

#include "api_mask.h"
#include "utils/exceptions.h"
#include "utils/keccak/keccak.h"

namespace stego_disk {

KeccakHashImpl::KeccakHashImpl(std::size_t state_size) {
    //TODO: check if stateSize is valid
    state_size_ = state_size;
}

KeccakHashImpl::~KeccakHashImpl() {}

void KeccakHashImpl::Process(MemoryBuffer& state,
                             const uint8* data, std::size_t length) {
    if (state_size_ != state.GetSize())
        throw std::length_error("KeccakHashImpl: input state size "
                                "doesnt match with current state size");

    if (data == nullptr)
        throw exception::NullptrArgument{"data"};

    //TOOD: rewrite implementation (int data types)
    //TODO: create new optimized version (sth like FastKeccakImpl / NISTKeccakImpl)
    keccak(data, static_cast<int>(length),
           state.GetRawPointer(), static_cast<int>(state_size_));
}

} // stego_disk
