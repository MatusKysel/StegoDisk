/**
* @file feistel_mix_permutation.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Feistel Mixed permutation implementation
*
*/

#include "feistel_mix_permutation.h"

#include <algorithm>

#include "hash/hash.h"
#include "logging/logger.h"
#include "utils/config.h"
#include "utils/exceptions.h"
#include "utils/keccak/keccak.h"
#include "utils/stego_errors.h"
#include "utils/stego_math.h"

#define FMP_MIN_REQ_SIZE                        1024
#define FMP_NUMROUNDS                           5

namespace stego_disk {

FeistelMixPermutation::FeistelMixPermutation() :
  left_bits_(0),
  left_mod_(0),
  right_mask_(0),
  right_bits_(0) {

  LOG_DEBUG("Permutation::Permutation: constructor called for: " <<
            GetNameInstance());
}

FeistelMixPermutation::~FeistelMixPermutation() {
  LOG_DEBUG("Permutation::~Permutation: destructor called for: " <<
            GetNameInstance());
}


void FeistelMixPermutation::Init(PermElem requested_size, Key &key) {
  if (key.GetSize() == 0)
    throw exception::EmptyArgument{"key"};

  if (requested_size < FMP_MIN_REQ_SIZE)
    throw std::invalid_argument("FeistelMixPermutation: "
                             "requested_size < FMP_MIN_REQ_SIZE (=1024)");

  uint8 bit_len = StegoMath::Log2(requested_size);

  if (bit_len < 8)
    throw std::invalid_argument("FeistelMixPermutation: "
                             "requested size is too small");

  initialized_ = false;

  left_bits_ = bit_len / 2;
  right_bits_ = bit_len - left_bits_;
  left_mod_ = (requested_size >> right_bits_);
  right_mask_ = (1 << right_bits_) - 1;

  size_ = left_mod_ << right_bits_;

  uint32 max_hash = static_cast<uint32>(right_mask_ + 1);

  hash_tables_ = std::vector<std::vector<uint32> >(
                   FMP_NUMROUNDS, std::vector<uint32>(max_hash, 0));

  //TODO: hash could be initialized_ by key and then just append "i" in each iteration - or not?
  //      sth like: Hash hash(key.getData());
  //      for (i..) hash.append(..)

  Hash hash;

  LOG_TRACE("FeistelMixPermutation::init: computing hash table (HT) containing "
            << max_hash << " elements");

  for (uint32 t = 0; t < FMP_NUMROUNDS; ++t) {
    for (uint32 i = 0; i < max_hash; ++i) {
      hash.Process(key.GetData());
      hash.Append((uint8*)&i, sizeof(uint32));
      hash.Append((uint8*)&t, sizeof(uint32));

      //TODO: rewrite these lines - im sure there is a better way of writing this

      if (hash.GetStateSize() < 4)
        throw exception::HashSizeTooSmall{};

      hash_tables_[t][i] = *((uint32*)hash.GetState().GetConstRawPointer()) % max_hash;
    }
  }
  LOG_TRACE("FeistelMixPermutation::init: HT ready; left_mod_ = "
            << left_mod_ << ", right_mask_ = " << right_mask_ << endl);

  initialized_ = true;
}

PermElem FeistelMixPermutation::Permute(PermElem index) const {
  CommonPermuteInputCheck(index);

  uint64 right = index & right_mask_;
  uint64 left = index >> right_bits_;

  for (int r = 0; r < FMP_NUMROUNDS; ++r) {
    if (r % 2) {
      right = (right ^ (hash_tables_[r][left] & right_mask_));
    } else {
      left = (left + (hash_tables_[r][right] >> right_bits_)) % left_mod_;
    }
  }

  uint64 permuted_index = (left << right_bits_) + right;

  if (permuted_index >= size_)
    throw exception::PermutationFailed{};

  return permuted_index;
}

PermElem FeistelMixPermutation::GetSizeUsingParams(PermElem requested_size,
                                                   Key& /*key*/) {
  if (requested_size < FMP_MIN_REQ_SIZE) return 0;

  uint8 bit_len = StegoMath::Log2(requested_size);

  if (bit_len < 8)
    return 0;

  uint8 right_bits = bit_len - bit_len / 2;
  return (requested_size >> right_bits) << right_bits;
}

} // stego_disk
