/**
* @file permutation.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Permutation interface
*
*/

#include "permutation.h"

#include <string.h> // memset

namespace stego_disk {

Permutation::Permutation() : size_(0), initialized_(false) {}

Permutation::~Permutation() {}

PermElem Permutation::GetSize() const {
  return size_;
}

void Permutation::CommonPermuteInputCheck(PermElem index) const {
  if (index >= size_)
    throw std::out_of_range("Permutation: element index out of range");

  if (!initialized_)
    throw std::runtime_error("Permutation: permutation must be initialized before use");
}

} // stego_disk
