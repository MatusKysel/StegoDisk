//
//  CarrierFileFactory.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 4/11/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "CarrierFile.h"
#include "CarrierFiles/CarrierFileBMP.h"
#include "CarrierFiles/CarrierFileJPEG.h"
#include "CarrierFiles/CarrierFileFactory.h"
#include "Encoders/EncoderFactory.h"
#include <algorithm>

using namespace std;

namespace StegoDisk {

CarrierFilePtr CarrierFileFactory::createCarrierFile(const File& file)
{
    //CarrierFilePtr carrierFile;
    shared_ptr<CarrierFile> carrierFile;
    std::shared_ptr<Encoder> encoder = std::shared_ptr<Encoder>(nullptr);
    
    // get ext
    string ext = file.getExtension();
    if (ext.compare(".bmp") == 0)
    {
        carrierFile = make_shared<CarrierFileBMP>(file, encoder);
    } else if (ext.compare(".jpg") == 0) {
        carrierFile = make_shared<CarrierFileJPEG>(file, encoder);
    }

    if (carrierFile) {
        if (carrierFile->getRawCapacity() < 1) {
            carrierFile = CarrierFilePtr();
        }
    }
    
    return carrierFile;
}

}
