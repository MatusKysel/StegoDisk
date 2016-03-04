/**
* @file carrier_file_bmp.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File with interfce for loading and saving BMP files
*
*/

#ifndef STEGODISK_CARRIERFILES_CARRIERFILEBMP_H_
#define STEGODISK_CARRIERFILES_CARRIERFILEBMP_H_

#include <stdint.h>

#include <iostream>
#include <string>

#include "carrier_file.h"
#include "logging/logger.h"

using namespace std;

namespace stego_disk {

class CarrierFileBMP : public CarrierFile {

public:
  CarrierFileBMP(File file,
                 std::shared_ptr<Encoder> encoder,
                 std::shared_ptr<Permutation> permutation,
                 std::unique_ptr<Fitness> fitness);

  int LoadFile();
  int SaveFile();


private:
  uint32 bmp_offset_;
  uint64 bmp_size_;
};

} // stego_disk

#endif // STEGODISK_CARRIERFILES_CARRIERFILEBMP_H_
