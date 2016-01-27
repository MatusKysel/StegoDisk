//
//  PrimeFieldPermutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_PERMUTATIONS_AFFINE64PERMUTATION_H_
#define STEGODISK_PERMUTATIONS_AFFINE64PERMUTATION_H_

#include "affine_permutation.h"

namespace stego_disk {

class Affine64Permutation : public AffinePermutation {

public:
  Affine64Permutation();
  ~Affine64Permutation();

  virtual void Init(PermElem requested_size, Key key);
  virtual PermElem Permute(PermElem index) const;

  const std::string GetNameInstance() const { return "Affine64"; }
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_AFFINE64PERMUTATION_H_
