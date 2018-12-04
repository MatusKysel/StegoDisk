/**
* @file keccak_hash_impl.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Implementation of hash function Keccak
*
*/

#pragma once

#include <memory>

#include "api_mask.h"
#include "hash_impl.h"
#include "utils/stego_header.h"
#include "utils/memory_buffer.h"

namespace stego_disk {

class KeccakHashImpl : public HashImpl {

public:
    KeccakHashImpl(std::size_t state_size = 32);
    virtual ~KeccakHashImpl();

	virtual void Process(MemoryBuffer& state, const uint8* data, std::size_t length) override;
};

} // stego_disk

