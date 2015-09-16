//
//  FeistelNumPermutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 4/15/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "feistel_num_permutation.h"

#include <math.h>       /* sqrt */
#include <algorithm>

#include "utils/stego_math.h"
#include "utils/keccak/keccak.h"
#include "utils/config.h"
#include "utils/stego_errors.h"
#include "logging/logger.h"
#include "hash/hash.h"

#define FNP_MIN_REQ_SIZE                        1024
#define FNP_NUMROUNDS                           5

namespace stego_disk {

FeistelNumPermutation::FeistelNumPermutation() : modulus_(0) {}

FeistelNumPermutation::~FeistelNumPermutation() {}


void FeistelNumPermutation::Init(PermElem requested_size, Key key) {
  if (key.GetSize() == 0)
    throw std::runtime_error("FeistelNumPermutation init: "
                             "Invalid key (size=0)");

  if (requested_size < FNP_MIN_REQ_SIZE)
    throw std::runtime_error("FeistelNumPermutation: "
                             "requestedSize < FNP_MIN_REQ_SIZE");

  initialized_ = false;

  modulus_ = static_cast<uint32>(sqrt(static_cast<double>(requested_size)));
  size_ = static_cast<size_t>((static_cast<uint64>(modulus_) *
                               static_cast<uint64>(modulus_)));

  // precompute hash table
  uint32 max_hash = modulus_;

  hash_tables_ = std::vector<std::vector<uint32> >
                 (FNP_NUMROUNDS, std::vector<uint32>(max_hash, 0));

  //TODO: hash could be initialized by key and then just append "i" in each iteration - or not?
  //      sth like: Hash hash(key.getData());
  //      for (i..) hash.append(..)

  Hash hash;

  for (uint32 t = 0; t < FNP_NUMROUNDS; ++t) {
    for (uint32 i = 0; i < max_hash; ++i) {
      hash.Process(key.GetData());
      hash.Append((uint8*)&i, 4);
      hash.Append((uint8*)&t, 4);

      //TODO: rewrite these lines - im sure there is a better way of writing this

      if (hash.GetStateSize() < 4)
        throw std::runtime_error("hash size is too small");

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
  for (std::size_t r=0; r<FNP_NUMROUNDS; r++) {
    if (r % 2) {
      right = (right + hash_tables_[r][left]) % modulus_;
    } else {
      left = (left + hash_tables_[r][right]) % modulus_;
    }
  }

  //    left = (left + hashTables[r++][right]) % modulus;
  //    right = (right + hashTables[r++][left]) % modulus;
  //    left = (left + hashTables[r++][right]) % modulus;
  //    right = (right + hashTables[r++][left]) % modulus;
  //    left = (left + hashTables[r++][right]) % modulus;

  uint64 permuted_index = (left * modulus_) + right;

  if (permuted_index >= size_)
    throw std::runtime_error("FeistelNumPermutation: "
                             "permuted index calculation failed");

  return permuted_index;
}

PermElem FeistelNumPermutation::GetSizeUsingParams(PermElem requested_size,
                                                   Key key) {
  if (requested_size < FNP_MIN_REQ_SIZE) return 0;

  uint32 mod = static_cast<uint32>(sqrt(static_cast<double>(requested_size)));
  return static_cast<std::size_t>(static_cast<uint64>(mod) *
                                  static_cast<uint64>(mod));
}


//const string FeistelNumPermutation::getNameInstance() const
//{
//    return getName();
//}
//
//const string FeistelNumPermutation::getName()
//{
//    return string("NumericFeistel");
//}
//
//shared_ptr<Permutation> FeistelNumPermutation::getNew()
//{
//    return shared_ptr<Permutation>(new FeistelNumPermutation());
//}
//
//shared_ptr<Permutation> FeistelNumPermutation::getNewInstance()
//{
//    return getNew();
//}

} // stego_disk
