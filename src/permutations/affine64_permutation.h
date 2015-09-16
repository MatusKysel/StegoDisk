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
  virtual void Init(PermElem requested_size, Key key);
  virtual PermElem Permute(PermElem index) const;
  //virtual PermElem getSizeUsingParams(PermElem requestedSize, Key key);

  virtual const std::string GetNameInstance() const { return "Affine64"; }

  //    const string getNameInstance() const;
  //    static const string getName();
  //    virtual shared_ptr<Permutation> getNewInstance();
  //    static shared_ptr<Permutation> getNew();
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_AFFINE64PERMUTATION_H_
