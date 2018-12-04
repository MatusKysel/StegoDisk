/**
* @file hash.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Interface for hash functions
*
*/

#pragma once

#include <memory>

#include "api_mask.h"
#include "hash_impl.h"
#include "utils/stego_header.h"
#include "utils/memory_buffer.h"

namespace stego_disk {

class Hash final {
public:
  Hash();
  Hash(const std::string& data);
  Hash(const uint8* data, std::size_t length);

  void Process(const std::string& data);
  void Process(const MemoryBuffer& data);
  void Process(const uint8* data, std::size_t length);

  void Append(const std::string& data);
  void Append(const MemoryBuffer& data);
  void Append(const uint8* data, std::size_t length);
  const MemoryBuffer& GetState() const;
  std::size_t GetStateSize() const;

private:
  void Init();
  MemoryBuffer state_;

  //TODO: implementation could be stored as shared ptr in each hash instance
  //  std::shared_ptr<HashImpl> hashImpl;
private:
  static std::unique_ptr<HashImpl> default_hash_impl_;

};

} // stego_disk

