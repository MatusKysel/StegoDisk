//
//  CarrierFileFactory.h
//  StegoFS
//
//  Created by Martin Kosdy on 4/11/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__CarrierFileFactory__
#define __StegoFS__CarrierFileFactory__

#include <iostream>
#include "Logging/logger.h"
#include "Utils/File.h"
#include <memory>

namespace StegoDisk {


#ifndef __SHARED_PTR_CARRIER_FILE__
#define __SHARED_PTR_CARRIER_FILE__
class CarrierFile;
typedef std::shared_ptr<CarrierFile> CarrierFilePtr;
#endif // __SHARED_PTR_CARRIER_FILE__

class CarrierFileFactory
{
public:
    static CarrierFilePtr createCarrierFile(const File& file);
};

};

#endif /* defined(__StegoFS__CarrierFileFactory__) */
