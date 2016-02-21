/**
* @file key.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Key class
*
*/

#ifndef STEGODISK_KEYS_KEY_H_
#define STEGODISK_KEYS_KEY_H_

#include <string>
#include <memory>

#include "utils/memory_buffer.h"

namespace stego_disk {

class Key {

public:
  Key();
  Key(const MemoryBuffer& data_buffer);
  ~Key();

  std::size_t GetSize();
  const MemoryBuffer& GetData() { return data_; }

  static Key FromString(std::string input);

  Key& operator^=(const Key& other);
  Key operator^(const Key& other);

private:
  MemoryBuffer data_;

};
} // stego_disk

#endif // STEGODISK_KEYS_KEY_H_
