#include "permutation_factory.h"

#include <algorithm>


#include "identity_permutation.h"
#include "affine_permutation.h"
#include "affine64_permutation.h"
#include "feistel_num_permutation.h"
#include "feistel_mix_permutation.h"

namespace stego_disk {


/**
 * @brief Get default permutation instance
 *
 * Method get default permutation instance.
 * This is FeistelMixPermutation class instance.
 *
 * @return Instance of default permutation.
 */
std::shared_ptr<Permutation> PermutationFactory::GetDefaultPermutation() {
  return GetPermutation(kDefaultPermutation);
}

PermutationFactory::PermutationType PermutationFactory::GetDefaultPermutationType() {
  return kDefaultPermutation;
}

/**
 * @brief Get vector of instances of all the permutations
 *
 * This method get vector of instances of all the permutations supported by this library.
 * Each permutation is in the vector only once.
 * initialized with default values of parameters (i.e. size of the permutation = 0).
 *
 * @return Vector of supported permutations
 */
vector<std::shared_ptr<Permutation>> PermutationFactory::GetPermutations() {
  vector<std::shared_ptr<Permutation>> list;

  list.push_back(make_shared<IdentityPermutation>());
  list.push_back(make_shared<AffinePermutation>());
  list.push_back(make_shared<Affine64Permutation>());
  list.push_back(make_shared<FeistelNumPermutation>());
  list.push_back(make_shared<FeistelMixPermutation>());

  return list;
}

/**
 * @brief Get instance of the permutation by the code name
 *
 * This method get instance of the permutation by the permutation's code name.
 * If those permutation exists, it is initialized with default parameters (i.e. size=0).
 * If there is no permutation with the 'permutationName', throws an exception.
 * Exception is raised if 'permutationName' has zero length, too.
 *
 * @param[in] permutationName Name of permutation to be created
 * @return On success instance of permutation, exception std::invalid_argument otherwise
 */
std::shared_ptr<Permutation> PermutationFactory::GetPermutation(
    const string &permutation_name) {

  vector<std::shared_ptr<Permutation> > list;
  std::shared_ptr<Permutation> permutation(nullptr);
  string str, name;

  if (permutation_name.length() == 0)
    throw std::invalid_argument("PermutationFactory::getPermutationByName: '"
                                "permutationName' is empty");
  name = permutation_name;

  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  list = GetPermutations();
  for (unsigned int i = 0; i < list.size(); ++i) {
    str = list[i]->GetNameInstance();
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    if (name.compare(str) == 0) {
      permutation = list[i];
      break;
    }
  }

  if (permutation)
    return permutation;
  throw std::invalid_argument("PermutationFactory::getPermutationByName: "
                              "permutation with name '"
                              + permutation_name + "' doesn't exist");
}


std::shared_ptr<Permutation> PermutationFactory::GetPermutation(
    const PermutationType permutation){

  switch(permutation) {
    case PermutationType::IDENTITY:
      return std::make_shared<IdentityPermutation>();
    case PermutationType::AFFINE:
      return std::make_shared<AffinePermutation>();
    case PermutationType::AFFINE64:
      return std::make_shared<Affine64Permutation>();
    case PermutationType::FEISTEL_NUM:
      return std::make_shared<FeistelNumPermutation>();
    case PermutationType::FEISTEL_MIX:
      return std::make_shared<FeistelMixPermutation>();
    default:
      return nullptr;
  }
}

PermutationFactory::PermutationType PermutationFactory::GetPermutationType(
    const std::string &permutation) {

  std::string l_perm(permutation.size(), '\0');
  std::transform(permutation.begin(), permutation.end(), l_perm.begin(), ::tolower);

  if (l_perm == "identity") {
    return PermutationType::IDENTITY;
  } else if (l_perm == "affine") {
    return PermutationType::AFFINE;
  } else if (l_perm == "affine64") {
    return PermutationType::AFFINE64;
  } else if (l_perm == "num_feistel") {
    return PermutationType::FEISTEL_NUM;
  } else if (l_perm == "mix_feistel"){
    return PermutationType::FEISTEL_MIX;
  } else {
    return GetDefaultPermutationType();
  }
}

const std::string PermutationFactory::GetPermutationName(
    const PermutationType permutation) {

  if (permutation == PermutationType::IDENTITY) {
    return "identity";
  } else if (permutation == PermutationType::AFFINE) {
    return "affine";
  } else if (permutation == PermutationType::AFFINE64) {
    return "affine64";
  } else if (permutation == PermutationType::FEISTEL_NUM) {
    return "num_feistel";
  } else {
    return "mix_feistel";
  }
}
} // stego_disk
