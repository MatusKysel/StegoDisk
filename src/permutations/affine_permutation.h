/**
* @file affine_permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Affine permutation
*
*/

#pragma once

#include "permutation.h"

namespace stego_disk {

class AffinePermutation : public Permutation {

public:
  AffinePermutation();
  ~AffinePermutation();

  virtual void Init(PermElem requested_size, Key &key) override;
  virtual PermElem Permute(PermElem index) const override;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key &key) override;

  const std::string GetNameInstance() const { return "Affine"; }
protected:
  PermElem GetSizeUsingParams(PermElem requested_size, Key &key,
                              bool overwrite_members);
  uint64 key_param_a_;
  uint64 key_param_b_;
};

} // stego_disk
