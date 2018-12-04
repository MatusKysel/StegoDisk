/**
* @file identity_permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Identity permutation
*
*/

#pragma once

#include "permutation.h"

namespace stego_disk {

class IdentityPermutation : public Permutation {

public:
  IdentityPermutation();
  ~IdentityPermutation();

  virtual void Init(PermElem requested_size, Key &key) override;
  virtual PermElem Permute(PermElem index) const override;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key &key) override;

  const std::string GetNameInstance() const override;
};

} // stego_disk
