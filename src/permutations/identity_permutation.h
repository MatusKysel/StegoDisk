/**
* @file identity_permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Identity permutation
*
*/

#ifndef STEGODISK_PERMUTATIONS_IDENTITYPERMUTATION_H_
#define STEGODISK_PERMUTATIONS_IDENTITYPERMUTATION_H_

#include "permutation.h"

namespace stego_disk {

class IdentityPermutation : public Permutation {

public:
  IdentityPermutation();
  ~IdentityPermutation();

  virtual void Init(PermElem requested_size, Key &key);
  virtual PermElem Permute(PermElem index) const;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key &key);

  const std::string GetNameInstance() const { return "Identity"; }
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_IDENTITYPERMUTATION_H_
