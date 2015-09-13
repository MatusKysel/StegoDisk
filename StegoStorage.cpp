//
//  StegoStorage.cpp
//  StegoDiskCore
//
//  Created by Martin Kosdy on 15/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//

#include "VirtualStorage/VirtualStorage.h"
#include "FileManagement/CarrierFilesManager.h"
#include "Encoders/EncoderFactory.h"
#include "StegoStorage.h"
#include "FileManagement/CarrierFilesManager.h"
#include "VirtualStorage/VirtualStorage.h"

using namespace std;

namespace StegoDisk {

StegoStorage::StegoStorage() :
    carrierFilesManager(new CarrierFilesManager()),
    opened(false)
{
}
    
StegoStorage::~StegoStorage()
{
    
}
    
void StegoStorage::open(std::string storageBasePath, std::string password)
{
    opened = false;
    
    carrierFilesManager->setPassword(password);
    carrierFilesManager->loadDirectory(storageBasePath);
    
    opened = true;
}
    
void StegoStorage::load()
{
    if (!opened)
        throw std::runtime_error("Storage must be opened before loading");
    
    // set encoder
    // TODO manager->setPermutation for carrier files
	carrierFilesManager->setEncoder(EncoderFactory::getEncoderByName("hamming"));
    //carrierFilesManager.setEncoder(EncoderFactory::getEncoderByName("lsb"));
    //carrierFilesManager.setEncoderArgByName("blockSize","1");
    carrierFilesManager->applyEncoder();
    
    virtualStorage = make_shared<VirtualStorage>();
    virtualStorage->setPermutation(PermutationFactory::getPermutationByName("MixedFeistel"));
    carrierFilesManager->loadVirtualStorage(virtualStorage);
}
    
void StegoStorage::save()
{
    if (!opened)
        throw std::runtime_error("Storage must be opened before saving");
    
    if (virtualStorage == nullptr)
        throw std::runtime_error("Storage must be loaded before saving");
    
    carrierFilesManager->saveVirtualStorage();
}

void StegoStorage::read(void* destination, std::size_t offset, std::size_t length) const
{
    if (virtualStorage == nullptr)
        throw std::runtime_error("Storage must be loaded before use");
    
    virtualStorage->read(offset, length, (uint8*) destination);
}

void StegoStorage::write(void* source, std::size_t offset, std::size_t length)
{
    if (virtualStorage == nullptr)
        throw std::runtime_error("Storage must be loaded before use");
    
    virtualStorage->write(offset, length, (uint8*) source);
}

std::size_t StegoStorage::getSize() const
{
    if (virtualStorage == nullptr)
        return 0;
    
    return virtualStorage->getUsableCapacity();
}

}
