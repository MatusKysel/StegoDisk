/**
* @file memory_buffer.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Memofry buffer implementation
*
*/

#include "memory_buffer.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <ctime>

#include "api_mask.h"
#include "stego_types.h"

using namespace std;

namespace stego_disk {

void MemoryBuffer::Init(std::size_t new_size) {
  size_ = new_size;
  if (size_ > 0) {
    buffer_ = new uint8[size_];
  } else {
    size_ = 0;
    buffer_ = nullptr;
  }
}

MemoryBuffer::MemoryBuffer() : buffer_(nullptr), size_(0) {}


MemoryBuffer::MemoryBuffer(std::size_t new_size) : size_(new_size) {
  Init( new_size );
}

MemoryBuffer::MemoryBuffer(const uint8* data, std::size_t length) {
  Init( length );

  if (size_ == 0) return;

  memcpy(buffer_, data, size_);
}

/*
 * copy constructor
 */
MemoryBuffer::MemoryBuffer(const MemoryBuffer& other) {
  Init( other.GetSize() );

  if (size_ == 0) return;

  memcpy(buffer_, other.buffer_, size_);
}

/*
 * move constructor
 * other buffer is not destroyed - data are moved to new buffer
 */
MemoryBuffer::MemoryBuffer(MemoryBuffer&& other) {
  if ( buffer_ != nullptr ) Destroy();

  buffer_ = other.buffer_;
  size_ = other.size_;

  other.buffer_ = nullptr;
  other.size_ = 0;
}

// copy assignment
MemoryBuffer& MemoryBuffer::operator=(const MemoryBuffer& other) {
  // self-assignment check expected
  if ( this == &other ) return *this;

  if (size_ != other.size_) {
    Destroy();
    Init(other.size_);
  }
  memcpy(buffer_, other.buffer_, size_);
  return *this;
}

// move assignment
MemoryBuffer& MemoryBuffer::operator=(MemoryBuffer&& other) {
  Destroy();
  buffer_ = other.buffer_;
  size_ = other.size_;
  other.size_ = 0;
  other.buffer_ = nullptr;
  return *this;
}

MemoryBuffer::~MemoryBuffer() {
  Destroy();
}

bool MemoryBuffer::operator==(const MemoryBuffer& other) {

  if (size_ != other.GetSize()) return false;

  if (size_ == 0) return true;

  if ((other.buffer_ == nullptr) || (buffer_ == nullptr)) return false;

  return !(memcmp(buffer_, other.buffer_, size_));
}

bool MemoryBuffer::operator!=(const MemoryBuffer& other) {
  return !(*this == other);
}


void MemoryBuffer::Resize(std::size_t new_size) {

  if (new_size == size_) return;

  uint8* original_buffer = buffer_;
  std::size_t original_size = size_;

  uint8* new_buffer = new uint8[new_size];

  memset(new_buffer, 0, new_size);
  if (original_buffer != nullptr) {
    memcpy(new_buffer, original_buffer, std::min(new_size, original_size));
    Randomize();
    delete[] original_buffer;
  }
  size_ = new_size;
  buffer_ = new_buffer;
}

uint8& MemoryBuffer::operator[](std::size_t index) {

  if (index >= size_)
    throw std::out_of_range("MemoryBuffer: index out of range");

  return buffer_[index];
}

const uint8& MemoryBuffer::operator[](std::size_t index) const {

  if (index >= size_)
    throw std::out_of_range("MemoryBuffer: index out of range");

  return buffer_[index];
}

MemoryBuffer& MemoryBuffer::operator^=(const MemoryBuffer& other) {

  if (size_ != other.GetSize())
    throw std::length_error("MemoryBuffer: size must be equal to use xor "
                            "operator");

  for (size_t i = 0; i < size_; ++i) (*this)[i] ^= other[i];

  return *this;
}

MemoryBuffer MemoryBuffer::operator^(const MemoryBuffer& other) {

  if (size_ != other.GetSize())
    throw std::length_error("MemoryBuffer: size must be equal to use xor "
                            "operator");

  MemoryBuffer result(*this);
  result ^= other;
  return result;
}

std::size_t MemoryBuffer::GetSize() const {
  return size_;
}

uint8* MemoryBuffer::GetRawPointer() const {
  return buffer_;
}

const uint8* MemoryBuffer::GetConstRawPointer() const {
  return buffer_;
}

void MemoryBuffer::Write(std::size_t offset, const uint8* data,
                         std::size_t length) {

  if ((offset + length) > size_)
    throw  std::out_of_range("MemoryBuffer: write: offset + length > size");

  memcpy(static_cast<void*>(buffer_ + offset), data, length);
}


void MemoryBuffer::Clear() {

  if ((buffer_ == nullptr) || (size_ == 0)) return;

  memset(buffer_, 0, size_);
}

void MemoryBuffer::Randomize() {

  if ((buffer_ == nullptr) || (size_ == 0)) return;

  memset(buffer_, 0, size_);
  memset(buffer_, 0xFF, size_); //TODO(Matus) naco je toto dobre

  srand(static_cast<unsigned int>(time(NULL)));
  for (std::size_t i = 0; i < size_; ++i) {
    buffer_[i] = static_cast<uint8>(rand()); //PSTODO naozaj? nepiseme nahodou kniznicu ktora sluzi na security...?
  }
}

void MemoryBuffer::Fill(uint8 value) {

  if ((buffer_ == nullptr) || (size_ == 0)) return;

  memset(buffer_, value, size_);
}

void MemoryBuffer::Destroy() {

  if ( buffer_ == nullptr ) return;

  Randomize();
  delete[] buffer_;
  buffer_ = nullptr;
  size_ = 0;
}

} // stego_disk
