/**
* @file affine64_permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Affine 64 permutation
*
*/

#pragma once

#include "affine_permutation.h"

namespace stego_disk {

class Affine64Permutation : public AffinePermutation {

public:
  Affine64Permutation();
  ~Affine64Permutation();

  virtual void Init(PermElem requested_size, Key &key) override;
  virtual PermElem Permute(PermElem index) const override;

  const std::string GetNameInstance() const { return "Affine64"; }
};

} // stego_disk
