/**
* @file feistel_mix_permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Feistel Mixed permutation
*
*/

#pragma once

#include <vector>

#include "permutation.h"

namespace stego_disk {

class FeistelMixPermutation : public Permutation {

public:
  FeistelMixPermutation();
  ~FeistelMixPermutation();

  virtual void Init(PermElem requested_size, Key &key) override;
  virtual PermElem Permute(PermElem index) const override;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key &key) override;

  const std::string GetNameInstance() const override;

  //    const string getNameInstance() const;
  //    static const string getName();
  //    virtual shared_ptr<Permutation> getNewInstance();
  //    static shared_ptr<Permutation> getNew();

private:

  std::vector< std::vector<uint32> > hash_tables_;

  uint8 left_bits_{ 0 };
  uint64 left_mod_{ 0 };

  uint64 right_mask_{ 0 };
  uint8 right_bits_{ 0 };
};

} // stego_disk
