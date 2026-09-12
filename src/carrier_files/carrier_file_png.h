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
#include <stdlib.h>

#include <iostream>
#include <memory>
#include <string>

#include "carrier_file.h"
#include "logging/logger.h"

#include "lodepng.h"

using namespace std;

namespace stego_disk {

class CarrierFilePNG : public CarrierFile {

public:
  CarrierFilePNG(File file, std::shared_ptr<Encoder> encoder,
                 std::shared_ptr<Permutation> permutation,
                 std::unique_ptr<Fitness> fitness);

  void LoadFile();
  void SaveFile();

private:
  // Decoding a PNG costs about as much as a third of re-encoding it, and
  // SaveFile() needs exactly the pixels LoadFile() already decoded. The
  // decoded image is kept between the two and released once it is written,
  // trading width * height * 3 bytes per loaded carrier for that second
  // decode. lodepng allocates with malloc, so the deleter matches.
  using DecodedImage = std::unique_ptr<unsigned char, void (*)(void *)>;

  // Decodes the carrier into decoded_image_, or leaves it in place if it is
  // already there.
  void DecodeInto(DecodedImage *image);

  // lodepng::State owns the PNG metadata and frees it with the carrier.
  lodepng::State state_;
  DecodedImage decoded_image_{nullptr, &free};
};

} // stego_disk

#endif // STEGODISK_CARRIERFILES_CARRIERFILEPNG_H_
