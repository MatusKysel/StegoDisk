//
//  CarrierFileJPEG.h
//  StegoFS
//
//  Created by Martin Kosdy on 1/9/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__CarrierFileJPEG__
#define __StegoFS__CarrierFileJPEG__

#include <iostream>
#include <string>
#include <stdint.h>
#include "Logging/logger.h"
#include "CarrierFiles/CarrierFile.h"

// jpeglib must be includes AFTER all precedense files
// because in VS2010 is included BaseTsd.h, which defines INT32 type
// which one conflicts with definition in jpeglib.h
#include <jpeglib.h>
#include <jerror.h>

using namespace std;

namespace StegoDisk {

class CarrierFileJPEG : public CarrierFile {

private:
	int computeCapacity();
    
public:
    CarrierFileJPEG(File file, std::shared_ptr<Encoder> encoder);

    int loadFile();
    int saveFile();

    int getHistogram();
    
};

}

#endif /* defined(__StegoFS__CarrierFileJPEG__) */
