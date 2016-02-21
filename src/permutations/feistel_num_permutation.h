/**
* @file feistel_num_permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Feistel Numerical permutation
*
*/

#ifndef STEGODISK_PERMUTATIONS_FEISTELNUMPERMUTATION_H_
#define STEGODISK_PERMUTATIONS_FEISTELNUMPERMUTATION_H_

#include <vector>

#include "permutation.h"
#include "utils/stego_types.h"


namespace stego_disk {

class FeistelNumPermutation : public Permutation {

public:
  FeistelNumPermutation();
  ~FeistelNumPermutation();

  virtual void Init(PermElem requested_size, Key key);
  virtual PermElem Permute(PermElem index) const;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key key);

  const std::string GetNameInstance() const { return "NumericFeistel"; }

private:
  std::vector< std::vector<uint32> > hash_tables_;

  uint32 modulus_;
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_FEISTELNUMPERMUTATION_H_
