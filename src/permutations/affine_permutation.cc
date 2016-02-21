/**
* @file affine_permutation.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Affine permutation implementation
*
*/

#include "affine_permutation.h"

#include "utils/stego_math.h"
#include "utils/stego_errors.h"
#include "utils/config.h"
#include "logging/logger.h"

namespace stego_disk {

AffinePermutation::AffinePermutation() {
  LOG_DEBUG("Permutation::Permutation: constructor called for: " <<
            GetNameInstance());
}

AffinePermutation::~AffinePermutation() {
  LOG_DEBUG("Permutation::~Permutation: destructor called for: " <<
            GetNameInstance());
}

PermElem AffinePermutation::GetSizeUsingParams(PermElem requested_size, Key &key,
                                               bool overwrite_members) {
  if (key.GetSize() == 0)
    throw std::runtime_error("AffinePermutation: Invalid key (size=0)");

  uint64 prime = requested_size;

  uint64 a, b;

  while(1) {

    prime = StegoMath::ClosestSmallerPrime(prime);
    if (prime < 1) {
      return 0;
    }

    a = 0;
    b = 0;

    // TODO: modify this to handle different key lengths
    if (key.GetSize() < 8)
      throw std::invalid_argument("AffinePermutation: "
                                  "key must be longer than 8 bytes");

    if ((key.GetSize() % 16) != 0)
      throw std::invalid_argument("AffinePermutation: "
                                  "wrong key length! should be "
                                  "nonzero multiple of 16");

    for (std::size_t i = 0; i < key.GetSize() / 2; i += 8) {
      a ^= (uint64)*(uint64*)(&key.GetData()[i]);
    }
    for (std::size_t i = key.GetSize() / 2; i < key.GetSize(); i += 8) {
      b ^= (uint64)*(uint64*)(&key.GetData()[i]);
    }

    a %= (prime / 2);
    b %= (prime / 2);

    a += (prime/2);
    b += (prime/2);

    if (StegoMath::Gcd(a, prime) == 1) {
      if (overwrite_members) {
        size_ = prime;
        key_param_a_ = a;
        key_param_b_ = b;
      }
      return prime;
    }
  }
}

PermElem AffinePermutation::GetSizeUsingParams(PermElem requested_size,
                                               Key &key) {
  if (requested_size > 0xFFFFFFFF) // max size = 2^32 blocks (due to slow mulmod aritmethics for numbers > 2^32)
    return 0;

  return GetSizeUsingParams(requested_size, key, false);
}


void AffinePermutation::Init(PermElem requested_size, Key &key)
{
  if (requested_size > 0xFFFFFFFF) // max size = 2^32 blocks (due to slow mulmod aritmethics for numbers > 2^32)
    throw std::out_of_range("AffinePermutation: "
                            "max requested size is (2^32)-1");

  initialized_ = false;

  if (GetSizeUsingParams(requested_size, key, true) == 0)
    throw std::runtime_error("AffinePermutation: requested size is too small");

  initialized_ = true;

  //LOG_DEBUG("AffinePermutation::init: hash is " << StegoMath::hexBufferToStr(keyHash, SFS_KEY_HASH_LENGTH) << ", size: " << _size << ", paramA: " << _key_param_a_ << ", paramB: " << _key_param_b_);
}


PermElem AffinePermutation::Permute(PermElem index) const {
  CommonPermuteInputCheck(index);
  return ((((index) * key_param_a_) % size_) + key_param_b_) % size_;
}

} // stego_disk
