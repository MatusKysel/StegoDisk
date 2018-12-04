/**
* @file feistel_num_permutation.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Feistel Numerical permutation implementation
*
*/

#include "feistel_num_permutation.h"

#include <math.h>       /* sqrt */

#include <algorithm>

#include "hash/hash.h"
#include "logging/logger.h"
#include "utils/config.h"
#include "utils/exceptions.h"
#include "utils/keccak/keccak.h"
#include "utils/stego_errors.h"
#include "utils/stego_math.h"

constexpr auto MinReqSize = 1024;
constexpr auto NumRounds = 5;

namespace stego_disk {

FeistelNumPermutation::FeistelNumPermutation() {
  LOG_DEBUG("Permutation::Permutation: constructor called for: " <<
            GetNameInstance());
}

FeistelNumPermutation::~FeistelNumPermutation() {
  LOG_DEBUG("Permutation::~Permutation: destructor called for: " <<
            GetNameInstance());
}


void FeistelNumPermutation::Init(PermElem requested_size, Key &key) {
  if (key.GetSize() == 0)
    throw exception::EmptyArgument{"key"};

  if (requested_size < MinReqSize)
    throw std::invalid_argument("FeistelNumPermutation: "
                             "requestedSize < FNP_MIN_REQ_SIZE");

  initialized_ = false;

  modulus_ = static_cast<uint32>(sqrt(static_cast<double>(requested_size)));
  size_ = static_cast<size_t>((static_cast<uint64>(modulus_) *
                               static_cast<uint64>(modulus_)));

  // precompute hash table
  uint32 max_hash = modulus_;

  hash_tables_ = std::vector<std::vector<uint32> >
                 (NumRounds, std::vector<uint32>(max_hash, 0));

  //TODO: hash could be initialized by key and then just append "i" in each iteration - or not?
  //      sth like: Hash hash(key.getData());
  //      for (i..) hash.append(..)

  Hash hash;

  for (uint32 t = 0; t < NumRounds; ++t) {
    for (uint32 i = 0; i < max_hash; ++i) {
      hash.Process(key.GetData());
      hash.Append((uint8*)&i, sizeof(uint32));
      hash.Append((uint8*)&t, sizeof(uint32));

      //TODO: rewrite these lines - im sure there is a better way of writing this

      if (hash.GetStateSize() < 4)
        throw exception::HashSizeTooSmall{};

      uint32 hash_val = *((uint32*)hash.GetState().GetConstRawPointer());
      hash_tables_[t][i] = hash_val % modulus_;
    }
  }

  initialized_ = true;
}

PermElem FeistelNumPermutation::Permute(PermElem index) const {
  CommonPermuteInputCheck(index);

  uint64 right = index % modulus_;
  uint64 left = index / modulus_;

  // feistel rounds
  for (std::size_t r = 0; r < NumRounds; ++r) {
    if (r % 2) {
      right = (right + hash_tables_[r][left]) % modulus_;
    } else {
      left = (left + hash_tables_[r][right]) % modulus_;
    }
  }

  uint64 permuted_index = (left * modulus_) + right;

  if (permuted_index >= size_)
    throw exception::PermutationFailed{};

  return permuted_index;
}

PermElem FeistelNumPermutation::GetSizeUsingParams(PermElem requested_size,
                                                   Key& /*key*/) {
  if (requested_size < MinReqSize) return 0;

  auto mod = static_cast<uint32>(sqrt(static_cast<double>(requested_size)));
  return static_cast<std::size_t>(static_cast<uint64>(mod) *
                                  static_cast<uint64>(mod));
}

const std::string FeistelNumPermutation::GetNameInstance() const
{
	return "NumericFeistel";
}

} // stego_disk
