#ifndef STEGOSYSTEMPARAMS_H
#define STEGOSYSTEMPARAMS_H

#include "Permutations/PermutationFactory.h"
#include "Encoders/EncoderFactory.h"
#include "FileManagement/CarrierFilesManager.h"
#include "Keys/Key.h"

namespace StegoDisk {

class StegoSystemParams
{

public:
    StegoSystemParams();
    ~StegoSystemParams();
    void destroyContent();
    static vector<StegoSystemParams*> getConfigurations(CarrierFilesManager* manager, Key key, uint64 minimalCapacity);

    PermutationPtr permutation;
    std::shared_ptr<Encoder> encoder;
    uint64 capacity;
};

}

#endif // STEGOSYSTEMPARAMS_H
