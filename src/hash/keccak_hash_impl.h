/**
* @file keccak_hash_impl.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Implementation of hash function Keccak
*
*/

#ifndef STEGODISK_HASH_KECCAKHASHIMPL_H_
#define STEGODISK_HASH_KECCAKHASHIMPL_H_

#include <memory>

#include "hash_impl.h"
#include "utils/stego_header.h"
#include "utils/memory_buffer.h"

namespace stego_disk {

class KeccakHashImpl : public HashImpl {

public:
    KeccakHashImpl(std::size_t state_size = 32);
    virtual ~KeccakHashImpl();

    virtual void Process(MemoryBuffer& state, const uint8* data, std::size_t length);
};

} // stego_disk

#endif // STEGODISK_HASH_KECCAKHASHIMPL_H_
