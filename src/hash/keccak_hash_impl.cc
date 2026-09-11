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
#include <string>

#include "api_mask.h"
#include "utils/exceptions.h"
#include "utils/keccak/keccak.h"

namespace stego_disk {

KeccakHashImpl::KeccakHashImpl(std::size_t state_size) {
    // The sponge rate is 200 - 2 * state_size bytes and is absorbed in whole
    // 64-bit words, so a size that does not leave a multiple of 8 would drop
    // the tail of every block. Requiring a multiple of 4 below 100 keeps the
    // rate word-aligned and positive.
    if (state_size == 0 || state_size >= 100 || (state_size % 4) != 0)
        throw std::invalid_argument(
            "KeccakHashImpl: unsupported state size "
            + std::to_string(state_size));

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

    if (keccak(data, static_cast<int>(length),
               state.GetRawPointer(), static_cast<int>(state_size_)) != 0)
        throw std::invalid_argument("KeccakHashImpl: invalid hash arguments");
}

} // stego_disk
