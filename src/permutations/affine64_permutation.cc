/**
* @file affine64_permutation.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Affine 64 permutation implementation
*
*/

#include "affine64_permutation.h"

#include "utils/stego_math.h"
#include "utils/stego_errors.h"
#include "utils/config.h"
#include "logging/logger.h"

namespace stego_disk {

Affine64Permutation::Affine64Permutation() {
  LOG_DEBUG("Permutation::Permutation: constructor called for: " <<
            GetNameInstance());
}

Affine64Permutation::~Affine64Permutation() {
  LOG_DEBUG("Permutation::~Permutation: destructor called for: " <<
            GetNameInstance());
}

void Affine64Permutation::Init(PermElem requested_size, Key key)
{
  initialized_ = false;

  if (GetSizeUsingParams(requested_size, key, true) == 0)
    throw std::runtime_error("Affine64Permutation: "
                             "requested size is too small");

  initialized_ = true;
}

PermElem Affine64Permutation::Permute(PermElem index) const {
  CommonPermuteInputCheck(index);

  return (StegoMath::Mulmod(index, key_param_a_, size_) + key_param_b_) % size_;
}

} // stego_disk


