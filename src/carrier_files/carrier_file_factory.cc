//
//  CarrierFileFactory.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 4/11/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "carrier_file_factory.h"

#include <algorithm>

#include "carrier_file.h"
#include "carrier_file_bmp.h"
#include "carrier_file_jpeg.h"
#include "encoders/encoder_factory.h"

using namespace std;

namespace stego_disk {

CarrierFilePtr CarrierFileFactory::CreateCarrierFile(const File& file) {
  //CarrierFilePtr carrier_file;
  shared_ptr<CarrierFile> carrier_file;
  std::shared_ptr<Encoder> encoder = std::shared_ptr<Encoder>(nullptr);

  // get ext
  string ext = file.GetExtension();
  if (ext.compare(".bmp") == 0) {
    carrier_file = make_shared<CarrierFileBMP>(file, encoder);
  } else if (ext.compare(".jpg") == 0) {
    carrier_file = make_shared<CarrierFileJPEG>(file, encoder);
  }

  if (carrier_file) {
    if (carrier_file->GetRawCapacity() < 1) {
      carrier_file = CarrierFilePtr();
    }
  }

  return carrier_file;
}

} // stego_disk
