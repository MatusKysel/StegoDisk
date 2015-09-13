//
//  StegoStorage.h
//  StegoDiskCore
//
//  Created by Martin Kosdy on 15/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#ifndef __StegoDiskCore__StegoStorage__
#define __StegoDiskCore__StegoStorage__

#include <memory>
#include <string>

namespace StegoDisk {

class CarrierFilesManager;
class VirtualStorage;

class StegoStorage {

public:
    StegoStorage();
    ~StegoStorage();
    
    void open(std::string storageBasePath, std::string password);
    void load();
    void save();
    
    void read(void* destination, std::size_t offset, std::size_t length) const;
    void write(void* source, std::size_t offset, std::size_t length);
    
    std::size_t getSize() const;
    
private:
	std::unique_ptr<CarrierFilesManager> carrierFilesManager;
    
    //TODO: after some refactoring unique_ptr could be enough here:
    std::shared_ptr<VirtualStorage> virtualStorage;
    
    bool opened;
    
};
    
}

#endif /* defined(__StegoDiskCore__StegoStorage__) */
