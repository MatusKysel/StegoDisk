//
//  Permutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_PERMUTATIONS_PERMUTATION_H_
#define STEGODISK_PERMUTATIONS_PERMUTATION_H_

#include <string>
#include <memory>
#include "utils/stego_types.h"
#include "keys/key.h"

namespace stego_disk {

typedef uint64 PermutationElement;
typedef PermutationElement PermElem;
typedef PermutationElement PermSize;

class Permutation {

public:
  Permutation();
  virtual ~Permutation();

  // abstract methods
  virtual void Init(PermElem requested_size, Key key) = 0;
  virtual PermElem Permute(PermElem index) const = 0;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key key) = 0;

  virtual const std::string GetNameInstance() const = 0;

  //TODO: not sure if this is a good practice
  //    static const string getName();
  //    virtual shared_ptr<Permutation> getNewInstance() = 0;

  // element access using [] operator
  PermElem& operator[](PermElem index);
  const PermElem& operator[](PermElem index) const;

  PermElem GetSize() const;

  bool IsInitialized() const { return initialized_; }

protected:
  void CommonPermuteInputCheck(PermElem index) const; // throws exceptions (out_of_range, runtime - not initialized)

  PermElem size_;
  bool initialized_;
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_PERMUTATION_H_
