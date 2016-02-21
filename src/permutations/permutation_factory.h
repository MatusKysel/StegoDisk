/**
* @file permutation_factory.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Permutation factory
*
*/

#ifndef STEGODISK_PERMUTATIONS_PERMUTATIONFACTORY_H_
#define STEGODISK_PERMUTATIONS_PERMUTATIONFACTORY_H_

#include <memory>
#include <vector>

#include "permutation.h"

using namespace std;

namespace stego_disk {

class PermutationFactory final {
private:
  PermutationFactory();
public:

  enum class PermutationType {
    IDENTITY,
    AFFINE,
    AFFINE64,
    FEISTEL_NUM,
    FEISTEL_MIX
  };

  // get vector of all permutations (each permutation once)
  static vector<std::shared_ptr<Permutation>> GetPermutations();
  // get instance of permutation based on the code name
  static std::shared_ptr<Permutation> GetPermutation(
      const string &permutation_name);
  // get instance of permutation based on type
  static std::shared_ptr<Permutation> GetPermutation(
      const PermutationType permutation);
  // get instance of the default permutation
  static std::shared_ptr<Permutation> GetDefaultPermutation();

  static PermutationType GetDefaultPermutationType();

  static PermutationType GetPermutationType(const std::string &permutation);

  static const std::string GetPermutationName( const PermutationType permutation);

private:
  static const PermutationType kDefaultPermutation = PermutationType::FEISTEL_MIX;
};

} // stego_disk

#endif // STEGODISK_PERMUTATIONS_PERMUTATIONFACTORY_H_
