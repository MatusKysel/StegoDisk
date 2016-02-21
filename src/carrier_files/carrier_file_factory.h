/**
* @file carrier_file_factory.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Factory for files
*
*/

#ifndef STEGODISK_CARRIERFILES_CARRIERFILEFACTORY_H_
#define STEGODISK_CARRIERFILES_CARRIERFILEFACTORY_H_

#include <iostream>
#include <memory>

#include "logging/logger.h"
#include "utils/file.h"

namespace stego_disk {


#ifndef __SHARED_PTR_CARRIER_FILE__
#define __SHARED_PTR_CARRIER_FILE__
class CarrierFile;
typedef std::shared_ptr<CarrierFile> CarrierFilePtr;
#endif // __SHARED_PTR_CARRIER_FILE__

class CarrierFileFactory {
public:
  static CarrierFilePtr CreateCarrierFile(const File& file);
};

} // stego_disk

#endif // STEGODISK_CARRIERFILES_CARRIERFILEFACTORY_H_
