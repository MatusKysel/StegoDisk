/**
* @file identity_permutation.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Identity permutation implementation
*
*/

#include "identity_permutation.h"

#include "utils/stego_header.h"

namespace stego_disk {

IdentityPermutation::IdentityPermutation() {
  LOG_DEBUG("Permutation::Permutation: constructor called for: " <<
            GetNameInstance());
}

IdentityPermutation::~IdentityPermutation() {
  LOG_DEBUG("Permutation::~Permutation: destructor called for: " <<
            GetNameInstance());
}

void IdentityPermutation::Init(PermElem requested_size, Key key) {
  //    if (key.getSize() == 0)
  //        throw std::runtime_error("IdentityPermutatuion init: Invalid key (size=0)");

  if (requested_size == 0)
    throw std::invalid_argument("IdentityPermutation: "
                                "requestedSize cannot be 0");

  size_ = requested_size;
  initialized_ = true;
}

PermElem IdentityPermutation::Permute(PermElem index) const {
  CommonPermuteInputCheck(index);

  return index;
}

PermElem IdentityPermutation::GetSizeUsingParams(PermElem requested_size,
                                                 Key key) {
  return requested_size;
}

} // stego_disk
