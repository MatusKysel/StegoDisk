/**
* @file carrier_file_factory.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Factory for files
*
*/

#include "carrier_file_factory.h"

#include <algorithm>

#include "carrier_file.h"
#include "carrier_file_bmp.h"
#include "carrier_file_jpeg.h"
#include "encoders/encoder_factory.h"
#include "permutations/permutation_factory.h"
#include "utils/stego_config.h"

using namespace std;

namespace stego_disk {

CarrierFilePtr CarrierFileFactory::CreateCarrierFile(const File& file) {
  //CarrierFilePtr carrier_file;
  shared_ptr<CarrierFile> carrier_file;
  std::shared_ptr<Encoder> encoder = std::shared_ptr<Encoder>(nullptr);

  // get ext
  string ext = file.GetExtension();
  if (ext.compare(".bmp") == 0) {
    carrier_file = make_shared<CarrierFileBMP>(file,
                                               encoder,
                                               PermutationFactory::GetPermutation(StegoConfig::local_perm()));
  } else if (ext.compare(".jpg") == 0) {
    carrier_file = make_shared<CarrierFileJPEG>(file,
                                                encoder,
                                                PermutationFactory::GetPermutation(StegoConfig::local_perm()));
  }

  if (carrier_file) {
    if (carrier_file->GetRawCapacity() < 1) {
      carrier_file = CarrierFilePtr();
    }
  }

  return carrier_file;
}

} // stego_disk
