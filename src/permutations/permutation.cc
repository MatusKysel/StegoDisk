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

Permutation::Permutation() {}

Permutation::~Permutation() {}

PermElem Permutation::GetSize() const {
  return size_;
}

void Permutation::CommonPermuteInputCheck(PermElem index) const {
  if (index >= size_)
    throw std::out_of_range("Permutation: element index out of range");

  if (!initialized_)
    exception::InvalidState{exception::Operation::commonPermuteInputCheck,
			                exception::Component::permutation,
							exception::ComponentState::notInitialized};
}

} // stego_disk
