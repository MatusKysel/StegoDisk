//
//  Key.cpp
//  StegoDisk
//
//  Created by Martin Kosdy on 4/14/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//
#include "keys/key.h"

#include <string.h> // memset

//#include "utils/stego_header.h"
#include "hash/hash.h"

//#define DEFAULT_KEY_SIZE        32

namespace stego_disk {

Key::Key() : data_(0) {}

Key::Key(const MemoryBuffer& data_buffer) : data_(data_buffer) {}

Key::~Key() {}

Key& Key::operator^=(const Key& other) {
    data_ ^= other.data_;
    return *this;
}

Key Key::operator^(const Key& other) {
    return Key(data_ ^ other.data_);
}

std::size_t Key::GetSize() {
    return data_.GetSize();
}

Key Key::FromString(std::string input) {
    return Key(Hash(input).GetState());
}

} // stego_disk

