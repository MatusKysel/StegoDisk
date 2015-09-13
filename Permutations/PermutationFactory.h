#ifndef PERMUTATIONFACTORY_H
#define PERMUTATIONFACTORY_H

#include <memory>

#include <vector>
//#include "Permutation.h"
using namespace std;

#include "Utils/Cpp11Compat.h"

namespace StegoDisk {

#ifndef __SHARED_PTR_PERMUTATION__
#define __SHARED_PTR_PERMUTATION__
    class Permutation;
    typedef std::shared_ptr<Permutation> PermutationPtr;
#endif // __SHARED_PTR_PERMUTATION__


class PermutationFactory KEYWORD_FINAL
{
private:
    PermutationFactory();
public:
    // get vector of all permutations (each permutation once)
    static vector<PermutationPtr> getPermutations();
    // get instance of permutation based on the code name
    static PermutationPtr getPermutationByName(const string &name);
    // get instance of the default permutation
    static PermutationPtr getDefaultPermutation();
};
    
}

#endif // PERMUTATIONFACTORY_H
