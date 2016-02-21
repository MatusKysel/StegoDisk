/**
* @file feistel_mix_permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Feistel Mixed permutation
*
*/

#ifndef STEGODISK_PERMUTATIONS_FEISTELMIXPERMUTATION_H_
#define STEGODISK_PERMUTATIONS_FEISTELMIXPERMUTATION_H_

#include <vector>

#include "permutation.h"

namespace stego_disk {

class FeistelMixPermutation : public Permutation {

public:
  FeistelMixPermutation();
  ~FeistelMixPermutation();

  virtual void Init(PermElem requested_size, Key key);
  virtual PermElem Permute(PermElem index) const;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key key);

  const std::string GetNameInstance() const { return "MixedFeistel"; }

  //    const string getNameInstance() const;
  //    static const string getName();
  //    virtual shared_ptr<Permutation> getNewInstance();
  //    static shared_ptr<Permutation> getNew();

private:

  std::vector< std::vector<uint32> > hash_tables_;

  uint8 left_bits_;
  uint64 left_mod_;

  uint64 right_mask_;
  uint8 right_bits_;
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_FEISTELMIXPERMUTATION_H_
