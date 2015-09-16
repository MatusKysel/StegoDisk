#ifndef STEGODISK_PERMUTATIONS_PERMUTATIONFACTORY_H_
#define STEGODISK_PERMUTATIONS_PERMUTATIONFACTORY_H_

#include <memory>
#include <vector>

#include "permutation.h"
#include "utils/cpp11_compat.h"

using namespace std;

namespace stego_disk {

#ifndef __SHARED_PTR_PERMUTATION__
#define __SHARED_PTR_PERMUTATION__
//class Permutation; // TODO (matus) aspon podla mna to tu nema byt, no mozno sa mylim
typedef std::shared_ptr<Permutation> PermutationPtr;
#endif // __SHARED_PTR_PERMUTATION__


class PermutationFactory KEYWORD_FINAL {
private:
  PermutationFactory();
public:
  // get vector of all permutations (each permutation once)
  static vector<PermutationPtr> GetPermutations();
  // get instance of permutation based on the code name
  static PermutationPtr GetPermutationByName(const string &permutation_name);
  // get instance of the default permutation
  static PermutationPtr GetDefaultPermutation();
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_PERMUTATIONFACTORY_H_
