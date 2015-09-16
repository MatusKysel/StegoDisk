//
//  HashImpl.h
//  StegoDiskCore
//
//  Created by Martin Kosdy on 14/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_HASH_HASHIMPL_H_
#define STEGODISK_HASH_HASHIMPL_H_

#include <memory>

#include "utils/stego_header.h"
#include "utils/memory_buffer.h"

namespace stego_disk {

class HashImpl {

public:
  HashImpl() : state_size_(0) {}
  virtual ~HashImpl() {}

  virtual void Process(MemoryBuffer& state,
                       const uint8* data, std::size_t length) = 0;
  virtual void Append(MemoryBuffer& state,
                      const uint8* data, std::size_t length);

  std::size_t GetStateSize() { return state_size_; }

protected:
  size_t state_size_;

};

} // stego_disk

#endif // STEGODISK_HASH_HASHIMPL_H_
