#include "Utils/StegoSystemParams.h"
#include "Permutations/Permutation.h"

namespace StegoDisk {

StegoSystemParams::StegoSystemParams()
{
    permutation = PermutationPtr(nullptr);
    encoder = std::shared_ptr<Encoder>(nullptr);
    capacity = 0;
}

StegoSystemParams::~StegoSystemParams()
{

}

void StegoSystemParams::destroyContent()
{
    if (permutation)
        permutation = PermutationPtr(nullptr);
    if (encoder)
        encoder = std::shared_ptr<Encoder>(nullptr);
}

vector<StegoSystemParams*> StegoSystemParams::getConfigurations(CarrierFilesManager *manager, Key key, uint64 minimalCapacity)
{
    vector<PermutationPtr> permutations = PermutationFactory::getPermutations();
    vector<std::shared_ptr<Encoder>> encoders = EncoderFactory::getAllEncoders();

    vector<StegoSystemParams*> paramsList;
    StegoSystemParams* params;
    uint64 encoderOutputCapacity;

    for (size_t p=0;p<permutations.size();p++) {
        for (size_t e=0;e<encoders.size();e++) {
            params = new (nothrow) StegoSystemParams();
            if (params == NULL) continue;

            params->permutation = permutations[p];
            params->encoder = encoders[e];

            encoderOutputCapacity = manager->getCapacityUsingEncoder(params->encoder);
            params->capacity = params->permutation->getSizeUsingParams(encoderOutputCapacity, key);
            
            if (params->capacity < minimalCapacity) {
                delete params;
                params = NULL;
            } else {
                paramsList.push_back(params);
            }

        }
    }

    return paramsList;

}

}
