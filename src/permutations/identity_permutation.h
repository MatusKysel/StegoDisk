//
//  IdentityPermutation.h
//  StegoFS
//
//  Created by Martin Kosdy on 3/22/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_PERMUTATIONS_IDENTITYPERMUTATION_H_
#define STEGODISK_PERMUTATIONS_IDENTITYPERMUTATION_H_

#include "permutation.h"

namespace stego_disk {

class IdentityPermutation : public Permutation {

public:
  IdentityPermutation();
  ~IdentityPermutation();

  virtual void Init(PermElem requested_size, Key key);
  virtual PermElem Permute(PermElem index) const;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key key);

  const std::string GetNameInstance() const { return "Identity"; }

  //    const string getNameInstance() const;
  //    static const string getName();
  //    virtual shared_ptr<Permutation> getNewInstance();
  //    static shared_ptr<Permutation> getNew();

};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_IDENTITYPERMUTATION_H_
