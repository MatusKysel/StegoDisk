//
//  FeistelMixPermutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 4/15/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "feistel_mix_permutation.h"

#include <algorithm>

#include "utils/stego_math.h"
#include "utils/keccak/keccak.h"
#include "utils/config.h"
#include "utils/stego_errors.h"
#include "logging/logger.h"
#include "hash/hash.h"

#define FMP_MIN_REQ_SIZE                        1024
#define FMP_NUMROUNDS                           5

namespace stego_disk {

FeistelMixPermutation::FeistelMixPermutation() :
  left_bits_(0),
  left_mod_(0),
  right_mask_(0),
  right_bits_(0) {

  LOG_DEBUG("Permutation::Permutation: called for: " << GetNameInstance());
}

FeistelMixPermutation::~FeistelMixPermutation() {
  LOG_DEBUG("Permutation::~Permutation: destructor called for: " <<
            GetNameInstance());
}


void FeistelMixPermutation::Init(PermElem requested_size, Key key) {
  if (key.GetSize() == 0)
    throw std::runtime_error("FeistelMixPermutation init: "
                             "Invalid key (size=0)");

  if (requested_size < FMP_MIN_REQ_SIZE)
    throw std::runtime_error("FeistelMixPermutation: "
                             "requested_size < FMP_MIN_REQ_SIZE (=1024)");

  uint8 bit_len = StegoMath::Log2(requested_size);

  if (bit_len < 8)
    throw std::runtime_error("FeistelMixPermutation: "
                             "requested size is too small");

  initialized_ = false;

  right_bits_ = bit_len / 2;
  left_bits_ = bit_len - right_bits_;
  left_mod_ = (requested_size >> right_bits_);
  right_mask_ = ((static_cast<uint64>(1)) << right_bits_) - 1;

  // we ensure that right_mask_ >= left_mod_
  if ( right_mask_ < left_mod_ ) {
    right_bits_++;
    left_bits_--;
    left_mod_ = (requested_size >> right_bits_);
    right_mask_ = ((static_cast<uint64>(1)) << right_bits_) - 1;
  }

  size_ = (left_mod_) << right_bits_;

  // precompute hash table

  // right_mask_ is always >= left_mod_
  uint32 max_hash = static_cast<uint32>(right_mask_);

  hash_tables_ = std::vector<std::vector<uint32> >(
                   FMP_NUMROUNDS, std::vector<uint32>(max_hash + 1, 0));

  //TODO: hash could be initialized_ by key and then just append "i" in each iteration - or not?
  //      sth like: Hash hash(key.getData());
  //      for (i..) hash.append(..)

  Hash hash;

  LOG_TRACE("FeistelMixPermutation::init: computing hash table (HT) containing "
            << max_hash + 1 << " elements");

  for (uint32 t = 0; t < FMP_NUMROUNDS; ++t) {
    for (uint64 i = 0; i <= static_cast<uint64>(max_hash); ++i) {
      uint32 index = static_cast<uint32>(i);
      hash.Process(key.GetData());
      hash.Append((uint8*)&index, sizeof(uint32));
      hash.Append((uint8*)&t, sizeof(uint32));

      //TODO: rewrite these lines - im sure there is a better way of writing this

      if (hash.GetStateSize() < 4)
        throw std::runtime_error("hash size is too small");

      uint32 hash_val = *((uint32*)hash.GetState().GetConstRawPointer());
      hash_tables_[t][index] = hash_val;
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

  // feistel rounds
  for (int r = 0; r < FMP_NUMROUNDS; ++r) {
    if (r % 2) {
      right = (right ^ (hash_tables_[r][left] & right_mask_));
    } else {
      left = (left + hash_tables_[r][right]) % left_mod_;
    }
  }

  uint64 permuted_index = (left << right_bits_) | right;

  if (permuted_index >= size_)
    throw std::runtime_error("FeistelMixPermutation: "
                             "permuted index calculation failed");

  return permuted_index;
}

PermElem FeistelMixPermutation::GetSizeUsingParams(PermElem requested_size,
                                                   Key key) {
  if (requested_size < FMP_MIN_REQ_SIZE) return 0;

  uint8 bit_len = StegoMath::Log2(requested_size);

  if (bit_len < 8)
    return 0;

  uint8 right_bits = bit_len / 2;
  uint64 left_mod = (requested_size >> right_bits);
  return (left_mod) << right_bits;
}


//const string FeistelMixPermutation::getNameInstance() const
//{
//    return getName();
//}
//
//const string FeistelMixPermutation::getName()
//{
//    return string("MixedFeistel");
//}
//
//shared_ptr<Permutation> FeistelMixPermutation::getNew()
//{
//    return shared_ptr<Permutation>(new FeistelMixPermutation());
//}
//
//shared_ptr<Permutation> FeistelMixPermutation::getNewInstance()
//{
//    return getNew();
//}

} // stego_disk
