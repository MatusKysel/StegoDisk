//
//  Permutation.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "permutation.h"

#include <string.h> // memset

namespace stego_disk {

Permutation::Permutation() : size_(0), initialized_(false) {}

Permutation::~Permutation() {}

PermElem Permutation::GetSize() const {
  return size_;
}

void Permutation::CommonPermuteInputCheck(PermElem index) const {
  if (index >= size_)
    throw std::out_of_range("Permutation: element index out of range");

  if (!initialized_)
    throw std::runtime_error("Permutation: permutation must be initialized before use");
}

} // stego_disk
