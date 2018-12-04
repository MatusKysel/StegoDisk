/**
* @file permutation.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Permutation interface
*
*/

#pragma once

#include <string>
#include <memory>
#include <stdexcept>

#include "utils/stego_types.h"
#include "keys/key.h"
#include "logging/logger.h"

namespace stego_disk {

using PermElem = uint64;

class Permutation {

public:
  Permutation();
  virtual ~Permutation();

  virtual void Init(PermElem requested_size, Key &key) = 0;
  virtual PermElem Permute(PermElem index) const = 0;
  virtual PermElem GetSizeUsingParams(PermElem requested_size, Key &key) = 0;
  virtual const std::string GetNameInstance() const = 0;

  PermElem GetSize() const;
  bool IsInitialized() const { return initialized_; }

protected:
  void CommonPermuteInputCheck(PermElem index) const; // throws exceptions (out_of_range, runtime - not initialized)

  PermElem size_{ 0 };
  bool initialized_{ false };
};

} // stego_disk
