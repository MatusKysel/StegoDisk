//
//  IdentityPermutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "identity_permutation.h"

#include "utils/stego_header.h"

namespace stego_disk {

IdentityPermutation::IdentityPermutation() {}

IdentityPermutation::~IdentityPermutation() {}

void IdentityPermutation::Init(PermElem requested_size, Key key) {
  //    if (key.getSize() == 0)
  //        throw std::runtime_error("IdentityPermutatuion init: Invalid key (size=0)");

  if (size_ == 0)
    throw std::invalid_argument("IdentityPermutation: "
                                "requestedSize cannot be 0");

  size_ = requested_size;
}

PermElem IdentityPermutation::Permute(PermElem index) const {
  CommonPermuteInputCheck(index);

  return index;
}

PermElem IdentityPermutation::GetSizeUsingParams(PermElem requested_size,
                                                 Key key) {
  return requested_size;
}


//const string IdentityPermutation::getNameInstance() const
//{
//    return getName();
//}
//
//const string IdentityPermutation::getName()
//{
//    return string("Identity");
//}
//
//shared_ptr<Permutation> IdentityPermutation::getNew()
//{
//    return shared_ptr<Permutation>(new IdentityPermutation());
//}
//
//shared_ptr<Permutation> IdentityPermutation::getNewInstance()
//{
//    return getNew();
//}


} // stego_disk
