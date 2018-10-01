/**
* @file permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Permutation interface
*
*/

#ifndef STEGODISK_PERMUTATIONS_PERMUTATION_H_
#define STEGODISK_PERMUTATIONS_PERMUTATION_H_

#include <string>
#include <memory>
#include <stdexcept>

#include "utils/stego_types.h"
#include "keys/key.h"
#include "logging/logger.h"

namespace stego_disk {

typedef uint64 PermutationElement;
typedef PermutationElement PermElem;

class Permutation {

public:
  Permutation();
  virtual ~Permutation();

  virtual void Init(PermElem requested_size, Key &key) = 0;
  virtual PermElem Permute(PermElem index) const = 0;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key &key) = 0;
  virtual const std::string GetNameInstance() const = 0;

  PermElem& operator[](PermElem index);
  const PermElem& operator[](PermElem index) const;
  PermElem GetSize() const;
  bool IsInitialized() const { return initialized_; }

protected:
  void CommonPermuteInputCheck(PermElem index) const; // throws exceptions (out_of_range, runtime - not initialized)

  PermElem size_{ 0 };
  bool initialized_{ false };
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_PERMUTATION_H_
