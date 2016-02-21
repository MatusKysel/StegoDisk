/**
* @file hash.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Interface for hash functions
*
*/

#include "hash.h"

#include <stdexcept>

#include "keccak_hash_impl.h"
#include "utils/stego_types.h"

namespace stego_disk {

std::unique_ptr<HashImpl> Hash::default_hash_impl_ = std::unique_ptr<HashImpl>(new KeccakHashImpl());


void Hash::Init() {
  if (default_hash_impl_ == nullptr)
    throw std::runtime_error("Hash: default hash implementation not set");

  state_.Resize(default_hash_impl_->GetStateSize());
  state_.Clear();
}

Hash::Hash() {
  Init();
}


Hash::Hash(const std::string& data) {
  Init();
  Process(data);
}

Hash::Hash(const uint8* data, std::size_t length) {
  Init();
  Process(data, length);
}

void Hash::Process(const std::string& data) {
  if (default_hash_impl_ == nullptr)
    throw std::runtime_error("Hash: default hash implementation not set");

  default_hash_impl_->Process(state_,
                              (uint8*)data.c_str(),
                              data.length());
}

void Hash::Process(const MemoryBuffer& data) {
  if (default_hash_impl_ == nullptr)
    throw std::runtime_error("Hash: default hash implementation not set");

  if (data.GetSize() == 0)
    throw std::invalid_argument("Hash: input data cannot be empty");

  default_hash_impl_->Process(state_,
                              data.GetConstRawPointer(),
                              data.GetSize());
}

void Hash::Process(const uint8* data, std::size_t length) {
  if (default_hash_impl_ == nullptr)
    throw std::runtime_error("Hash: default hash implementation not set");

  default_hash_impl_->Process(state_, data, length);
}

void Hash::Append(const std::string& data) {
  if (default_hash_impl_ == nullptr)
    throw std::runtime_error("Hash: default hash implementation not set");

  default_hash_impl_->Append(state_,
                             (uint8*)data.c_str(),
                             data.length());
}

void Hash::Append(const uint8* data, std::size_t length) {
  if (default_hash_impl_ == nullptr)
    throw std::runtime_error("Hash: default hash implementation not set");

  default_hash_impl_->Append(state_, data, length);
}

void Hash::Append(const MemoryBuffer& data) {
  if (default_hash_impl_ == nullptr)
    throw std::runtime_error("Hash: default hash implementation not set");

  default_hash_impl_->Append(state_, data.GetConstRawPointer(), data.GetSize());
}

const MemoryBuffer& Hash::GetState() const {
  return state_;
}

std::size_t Hash::GetStateSize() const {
  return state_.GetSize();
}

} // stego_disk
