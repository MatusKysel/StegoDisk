/**
* @file feistel_num_permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Feistel Numerical permutation
*
*/

#pragma once

#include <vector>

#include "permutation.h"
#include "utils/stego_types.h"

namespace stego_disk {

class FeistelNumPermutation : public Permutation {

public:
  FeistelNumPermutation();
  ~FeistelNumPermutation();

  virtual void Init(PermElem requested_size, Key &key) override;
  virtual PermElem Permute(PermElem index) const override;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key &key) override;

  const std::string GetNameInstance() const { return "NumericFeistel"; }

private:
  std::vector< std::vector<uint32> > hash_tables_;

  uint32 modulus_{ 0 };
};

} // stego_disk
