/**
* @file memory_buffer.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Memofry buffer
*
*/

#ifndef STEGODISK_UTILS_MEMORYBUFFER_H_
#define STEGODISK_UTILS_MEMORYBUFFER_H_

#include <string>

#include "stego_types.h"

namespace stego_disk {

class MemoryBuffer {
public:
  MemoryBuffer();
  MemoryBuffer(std::size_t size);
  MemoryBuffer(const uint8* data, std::size_t length);

  MemoryBuffer(const MemoryBuffer& other); // copy constructor
  MemoryBuffer(MemoryBuffer&& other); // move constructor

  MemoryBuffer& operator=(const MemoryBuffer& other); // copy assignment
  MemoryBuffer& operator=(MemoryBuffer&& other); // move assignment

  bool operator==(const MemoryBuffer& other);
  bool operator!=(const MemoryBuffer& other);

  ~MemoryBuffer();

  uint8& operator[](std::size_t index);
  const uint8& operator[](std::size_t index) const;

  MemoryBuffer& operator^=(const MemoryBuffer& other);
  MemoryBuffer operator^(const MemoryBuffer& other);

  void Write(std::size_t offset, const uint8* data, std::size_t length);

  std::size_t GetSize() const;
  void Resize(std::size_t new_size);
  uint8* GetRawPointer() const;
  const uint8* GetConstRawPointer() const;

  void Clear(); // set content to zero
  void Randomize(); // replace content by random data
  void Fill(uint8 value); // fill entire buffer with value

private:
  void Destroy();
  void Init(std::size_t new_size);

  uint8* buffer_;
  std::size_t size_;
};

} // stego_disk

#endif // STEGODISK_UTILS_MEMORYBUFFER_H_
