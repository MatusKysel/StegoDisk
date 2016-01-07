//
//  Affine64Permutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

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



//const string Affine64Permutation::GetNameInstance() const
//{
//    return GetName();
//}
//
//const string Affine64Permutation::GetName()
//{
//    return string("Affine64");
//}
//
//shared_ptr<Permutation> Affine64Permutation::GetNew()
//{
//    return shared_ptr<Permutation>(new Affine64Permutation());
//}
//
//shared_ptr<Permutation> Affine64Permutation::GetNewInstance()
//{
//    return GetNew();
//}

} // stego_disk


