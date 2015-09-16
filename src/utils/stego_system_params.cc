#include "stego_system_params.h"

#include "permutations/permutation.h"

namespace stego_disk {

StegoSystemParams::StegoSystemParams() {
    permutation_ = PermutationPtr(nullptr);
    encoder_ = std::shared_ptr<Encoder>(nullptr);
    capacity_ = 0;
}

StegoSystemParams::~StegoSystemParams() {}

void StegoSystemParams::DestroyContent() {

    if (permutation_) permutation_ = PermutationPtr(nullptr);
    if (encoder_) encoder_ = std::shared_ptr<Encoder>(nullptr);
}

std::vector<StegoSystemParams*> StegoSystemParams::GetConfigurations(
    CarrierFilesManager *manager, Key key, uint64 minimal_capacity) {

    std::vector<PermutationPtr> permutations =
        PermutationFactory::GetPermutations();
    std::vector<std::shared_ptr<Encoder>> encoders =
        EncoderFactory::GetAllEncoders();

    std::vector<StegoSystemParams*> params_list;
    StegoSystemParams* params;
    uint64 encoder_output_capacity;

    for (size_t p = 0; p < permutations.size(); ++p) {
        for (size_t e = 0; e < encoders.size(); ++e) {
            params = new (nothrow) StegoSystemParams();
            if (params == NULL) continue;

            params->permutation_ = permutations[p];
            params->encoder_ = encoders[e];

            encoder_output_capacity = manager->GetCapacityUsingEncoder(
                                        params->encoder_);
            params->capacity_ = params->permutation_->GetSizeUsingParams(
                                  encoder_output_capacity, key);

            if (params->capacity_ < minimal_capacity) {
                delete params;
                params = NULL;
            } else {
                params_list.push_back(params);
            }
        }
    }
    return params_list;
}

} // stego_disk
