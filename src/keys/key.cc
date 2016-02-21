/**
* @file key.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Key class implementation
*
*/

#include "keys/key.h"

#include <string.h> // memset

#include "hash/hash.h"

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

