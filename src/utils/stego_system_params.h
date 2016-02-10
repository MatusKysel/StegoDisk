#ifndef STEGODISK_UTILS_STEGOSYSTEMPARAMS_H_
#define STEGODISK_UTILS_STEGOSYSTEMPARAMS_H_

#include <vector>
#include <memory>

#include "permutations/permutation_factory.h"
#include "encoders/encoder_factory.h"
#include "file_management/carrier_files_manager.h"
#include "keys/key.h"

namespace stego_disk {

class StegoSystemParams {

public:
    StegoSystemParams();
    ~StegoSystemParams();
    void DestroyContent();
    static std::vector<StegoSystemParams*> GetConfigurations(
        CarrierFilesManager* manager, Key key, uint64 minimal_capacity);

    std::shared_ptr<Permutation> permutation_;
    std::shared_ptr<Encoder> encoder_;
    uint64 capacity_;
};

} // stego_disk

#endif // STEGODISK_UTILS_STEGOSYSTEMPARAMS_H_
