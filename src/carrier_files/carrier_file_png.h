/**
* @file carrier_file_png.h
* @author Matus Kysel
* @date 2016
* @brief File with interfce for loading and saving PNG files
*
*/

#ifndef STEGODISK_CARRIERFILES_CARRIERFILEPNG_H_
#define STEGODISK_CARRIERFILES_CARRIERFILEPNG_H_

#include <stdint.h>

#include <iostream>
#include <string>

#include "carrier_file.h"
#include "logging/logger.h"

#include "lodepng.h"

using namespace std;

namespace stego_disk {

class CarrierFilePNG : public CarrierFile {

public:
  CarrierFilePNG(File file,
                 std::shared_ptr<Encoder> encoder,
                 std::shared_ptr<Permutation> permutation,
                 std::unique_ptr<Fitness> fitness);

  void LoadFile();
  void SaveFile();

private:
  LodePNGState state_;
};

} // stego_disk

#endif // STEGODISK_CARRIERFILES_CARRIERFILEPNG_H_
