/**
* @file carrier_file_jpeg.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Class with JPEG file support
*
*/

#pragma once

#include <iostream>
#include <string>
#include <stdint.h>

#include "carrier_file.h"
#include "logging/logger.h"


// jpeglib must be includes AFTER all precedense files
// because in VS2010 is included BaseTsd.h, which defines INT32 type
// which one conflicts with definition in jpeglib.h
#include <jpeglib.h>
#include <jerror.h>

namespace stego_disk {

class CarrierFileJPEG : public CarrierFile {

private:
  void ComputeCapacity();

public:
  CarrierFileJPEG(File file,
                  std::shared_ptr<Encoder> encoder,
                  std::shared_ptr<Permutation> permutation,
                  std::unique_ptr<Fitness> fitness);

  virtual void LoadFile() override;
  virtual void SaveFile() override;

  int GetHistogram();

};

} // stego_disk