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

#include "utils/exceptions.h"

namespace stego_disk {

Permutation::Permutation() : size_(0), initialized_(false) {}

Permutation::~Permutation() {}

PermElem Permutation::GetSize() const {
  return size_;
}

void Permutation::CommonPermuteInputCheck(PermElem index) const {
  // Checked first: the range of an uninitialized permutation is meaningless,
  // and size_ == 0 would otherwise mask this state behind an out_of_range.
  if (!initialized_)
    throw exception::InvalidState{exception::Operation::commonPermuteInputCheck,
                                  exception::Component::permutation,
                                  exception::ComponentState::notInitialized};

  if (index >= size_)
    throw std::out_of_range("Permutation: element index out of range");
}

} // stego_disk
