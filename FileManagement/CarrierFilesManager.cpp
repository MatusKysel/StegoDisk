//
//  CarrierFilesManager.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 1/6/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include "FileManagement/CarrierFilesManager.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
//#include <dirent.h>
//#include <QtCore>

#include "CarrierFiles/CarrierFileFactory.h"
//#include "../CarrierFiles/CarrierFileBMP.h"
#include "CarrierFiles/CarrierFileJPEG.h"
#include "Utils/Keccak/keccak.h"
#include "Utils/StegoErrors.h"
#include "Utils/Config.h"
#include "Utils/StegoMath.h"

#include <stdio.h>
#include <stdlib.h>

//#include <QThreadPool>
//#include <unistd.h>

#include "Logging/logger.h"

#include "Utils/File.h"
#include "Utils/Cpp11Compat.h"

using namespace std;

namespace StegoDisk {

CarrierFilesManager::CarrierFilesManager()
{
    _capacity = 0;
    _filesInDirectory = 0;
    _loadingProgress = 0;
    _virtualStorage = VirtualStoragePtr(nullptr);
    _encoder = std::shared_ptr<Encoder>(nullptr);
    _isActiveEncoder = false;
    //_progressCallback = NULL;
//    _delegate = NULL;
}

CarrierFilesManager::~CarrierFilesManager()
{
    _carrierFiles.clear();
}

//void CarrierFilesManager::on_fileInitFinished()
//{
//    int loadingProgress;
//    _loadingProgressMutex.lock();
//    loadingProgress = ++_loadingProgress;
//    _loadingProgressMutex.unlock();

//    if (_delegate) {
//        _delegate->cfmProgressUpdated(loadingProgress, _filesInDirectory, CountingCapacity);
//    }
//}


//void CarrierFilesManager::on_fileLoadFinished()
//{
//    int loadingProgress;
//    _loadingProgressMutex.lock();
//    loadingProgress = ++_loadingProgress;
//    _loadingProgressMutex.unlock();

//    if (_delegate) {
//        _delegate->cfmProgressUpdated(loadingProgress, _carrierFiles.size(), LoadingFiles);
//    }
//}


int CarrierFilesManager::loadDirectory(std::string directory)
{
//    _loadingThreadPool.waitForDone();

    _carrierFiles.clear();
    _capacity = 0;
    _filesInDirectory = 0;

    _basePath = directory;

    vector<File> files = File::getFilesInDir(directory, "");
    
    _filesInDirectory = files.size();



	//LOG_TRACE("CarrierFilesManager::loadDirectory: dir enumeration results begin:");
	FOREACH( File, files, file ) {
		LOG_TRACE(FOREACH_ELM(file).getBasePath() + " - " + FOREACH_ELM(file).getRelativePath());
	}
	//LOG_TRACE("CarrierFilesManager::loadDirectory: dir enumeration results end");



    /*
    CarrierFileInitTask *task = new CarrierFileInitTask(_basePath, QDir(_basePath).relativeFilePath(sFilePath), &_carrierFiles, &_fileListMutex);
    QObject::connect(task, SIGNAL(finished()), this, SLOT(on_fileInitFinished()));
    _loadingThreadPool.start(task);
*/
	FOREACH( File, files, file ) {
        CarrierFilePtr carrierFile = CarrierFileFactory::createCarrierFile(FOREACH_ELM(file));
        if (carrierFile != nullptr) {
            _carrierFiles.push_back(carrierFile);
        }
    }

    for (uint64 i=0;i<_carrierFiles.size();i++) {
        LOG_TRACE("CarrierFilesManager::loadDirectory: '" << _carrierFiles[i]->getFile().getRelativePath() << "' has raw capacity " << _carrierFiles[i]->getRawCapacity());
    }

    std::sort(_carrierFiles.begin(), _carrierFiles.end(), CarrierFile::compareBySharedPointers);

    return STEGO_NO_ERROR;
}

// return false, if checksum is not valid, true otherwise
// TODO mY check PERMUTATION init by PASSWORD
bool CarrierFilesManager::loadVirtualStorage(VirtualStoragePtr storage)
{
    if (!storage)
        throw std::invalid_argument("CarrierFilesManager::loadVirtualStorage: arg 'storage' is nullptr");
    if (!_encoder)
        throw std::invalid_argument("CarrierFilesManager::loadVirtualStorage: encoder is not set yet");
    if (!_isActiveEncoder)
        throw std::invalid_argument("CarrierFilesManager::loadVirtualStorage: encoder is not applied yet");

    // mY moved to CFM::applyEnc, we need call CF::setSubkey before CF::setEnc
//    generateMasterKey();
//    deriveSubkeys();

    try { storage->applyPermutation(this->getCapacity(), masterKey); }
    catch (...) { throw; }

    uint64 offset = 0;

    uint64 remainingCapacity = storage->getRawCapacity();

    uint64 bytesUsed;

    //_loadingThreadPool.waitForDone();
    _loadingProgress = 0;

    for (size_t i=0;i<_carrierFiles.size();i++) {
        if (remainingCapacity > _carrierFiles[i]->getCapacity()) {
            remainingCapacity -= _carrierFiles[i]->getCapacity();
            bytesUsed = _carrierFiles[i]->getCapacity();
        } else {
            bytesUsed = remainingCapacity;
            remainingCapacity = 0;
        }
        _carrierFiles[i]->addToVirtualStorage(storage, offset, bytesUsed);
        offset += _carrierFiles[i]->getCapacity();

        /*
        if (_progressCallback != NULL) {
            _progressCallback(i+1, _carrierFiles.size(), LoadingFiles);
        }
        */
        /*
        if (_delegate) {
            _delegate->progressUpdated(i+1, _carrierFiles.size(), LoadingFiles);
        }
        */
    }

    for (size_t i=0;i<_carrierFiles.size();i++) {
        _carrierFiles[i]->loadFile();
//        CarrierFileLoadTask *task = new CarrierFileLoadTask(_carrierFiles[i]);
//        QObject::connect(task, SIGNAL(finished()), this, SLOT(on_fileLoadFinished()));
//        _loadingThreadPool.start(task);
    }

//    _loadingThreadPool.waitForDone();

    _virtualStorage = storage;
    try { 
        if ( _virtualStorage->isValidChecksum() == false ) {
            LOG_DEBUG("Data integrity test: checksum is NOT valid");
            return false;
        }
    } catch (...) { throw; }
    LOG_DEBUG("Data integrity test: checksum is valid");

    return true;
}

int CarrierFilesManager::saveVirtualStorage()
{    
    if (!_virtualStorage) return SE_UNINITIALIZED;

    _virtualStorage->writeChecksum();

    saveAllFiles();

    return STEGO_NO_ERROR;
}

void CarrierFilesManager::setEncoderArgByName(const string &param, const string &val)
{
    if (!_encoder)
	throw std::invalid_argument("CarrierFilesManager::setEncoderArgByName: encoder is not set yet");
    if (_isActiveEncoder)
        throw std::invalid_argument("CarrierFilesManager::setEncoderArgByName: another encoder is active; please first unset this encoder");

    try { EncoderFactory::setEncoderArgByName(_encoder,param,val); }
    catch (...) { throw; }
}

void CarrierFilesManager::unsetEncoder()
{
    for (size_t i=0;i<_carrierFiles.size();i++)
        _carrierFiles[i]->unsetEncoder();
    _encoder = std::shared_ptr<Encoder>(nullptr);
    _isActiveEncoder = false;
}

void CarrierFilesManager::setEncoder(std::shared_ptr<Encoder> encoder)
{
    if (!encoder)
	throw std::invalid_argument("CarrierFilesManager::setEncoder: input arg 'encoder' is nullptr");
    if (_encoder)
        unsetEncoder();
    _encoder = encoder;
    _isActiveEncoder = false;
}

void CarrierFilesManager::applyEncoder()
{
    if (!_encoder)
	throw std::invalid_argument("CarrierFilesManager::applyEncoder: encoder is not set yet");

    uint64 capacity = 0;
    uint64 rawCap = 0;

    // mY from CFM::loadVS
    generateMasterKey();
    deriveSubkeys();

    for (size_t i=0;i<_carrierFiles.size();i++) {
        _carrierFiles[i]->setEncoder(_encoder);
        capacity += _carrierFiles[i]->getCapacity();
	rawCap += _carrierFiles[i]->getRawCapacity();
    LOG_DEBUG("CarrierFilesManager::applyEncoder: file '" << _carrierFiles[i]->getFile().getRelativePath() << "': raw=" << _carrierFiles[i]->getRawCapacity() << ", cap=" << _carrierFiles[i]->getCapacity());
    }
    if (capacity == 0)
        throw std::out_of_range("CarrierFilesManager::applyEncoder: not enough stego space in carrier files");

    _capacity = capacity;
    _isActiveEncoder = true;
    LOG_DEBUG("CarrierFilesManager::applyEncoder: all carrier files sum: rawCap=" << rawCap << ", cap=" << capacity);
}


void CarrierFilesManager::setPassword(std::string password)
{
    passwordHash.process(password);
    LOG_DEBUG("CarrierFilesManager::setPassword: setting password: '" << password << "'");
}


/**
 * @brief Generates Master Key hash from password (hash) and keys generated from individual carrier files
 * @return Error code (0 = NO ERROR)
 */
void CarrierFilesManager::generateMasterKey()
{
    if (_carrierFiles.size() < 1) {
        LOG_ERROR("Nothing to hash, no files loaded...");
        //TODO: throw exception?
        return;
    }

    LOG_DEBUG("CarrierFilesManager::generateMasterKey: PSWD HASH is " << StegoMath::hexBufferToStr(passwordHash.getState()));

    Hash masterKeyHash(passwordHash);
    
    // add keys generated from params of individual carrier files
    for (size_t i=0; i<_carrierFiles.size(); i++) {
        masterKeyHash.append(_carrierFiles[i]->getPermKey().getData());
    }
    
    this->masterKey = Key(masterKeyHash.getState());
}

void CarrierFilesManager::deriveSubkeys()
{
    if (_carrierFiles.size() < 1) {
        LOG_ERROR("There is no carrier loaded...");
        return;
    }

    LOG_DEBUG("CarrierFilesManager::deriveSubkeys: master key is " << StegoMath::hexBufferToStr(masterKey.getData()));

    Hash hash;
    
    for (uint64 i=0;i<_carrierFiles.size();i++) {
        
        // subkey = hash ( hash(master_key | file_index) | hash(file_path) )

        hash.process(masterKey.getData());
        hash.append(std::to_string(i));
        hash.append(_carrierFiles[i]->getFile().getNormalizedPath());
        
        _carrierFiles[i]->setSubkey(Key(hash.getState()));

        LOG_DEBUG("CarrierFilesManager::deriveSubkeys: subkey for carrier '" << _carrierFiles[i]->getFile().getAbsolutePath() << "' is " << StegoMath::hexBufferToStr(hash.getState()));
    }

}


void CarrierFilesManager::saveAllFiles()
{
    for (size_t i=0;i<_carrierFiles.size();i++) {
        _carrierFiles[i]->saveFile();
//        if (_delegate) {
////            _delegate->cfmProgressUpdated(i+1, _carrierFiles.size(), SavingFiles);
//        }
    }
}


uint64 CarrierFilesManager::getCapacity()
{
    if (!_encoder)
        throw std::invalid_argument("CarrierFilesManager::getCapacity: encoder is not set yet");
    if (!_isActiveEncoder)
        throw std::invalid_argument("CarrierFilesManager::getCapacity: encoder is not applied yet");
    return _capacity;
}

uint64 CarrierFilesManager::getRawCapacity()
{
    uint64 capacity = 0;
    for (size_t i=0;i<_carrierFiles.size();i++) {
        capacity += _carrierFiles.at(i)->getRawCapacity();
    }
    return capacity;
}

/*
void CarrierFilesManager::setProgressCallback(ProgressCallbackFunction progressCallback)
{
    _progressCallback = progressCallback;
}
*/

//void CarrierFilesManager::setDelegate(CarrierFilesManagerDelegate *delegate)
//{
//    _delegate = delegate;
//}


uint64 CarrierFilesManager::getCapacityUsingEncoder(std::shared_ptr<Encoder> encoder)
{
    if (!encoder)
        throw std::invalid_argument("CarrierFilesManager::getCapacity: arg 'encoder' is nullptr");
    uint64 capacity = 0;
    for (size_t i=0;i<_carrierFiles.size();i++) {
        capacity += _carrierFiles.at(i)->getCapacityUsingEncoder(encoder);
    }
    return capacity;
}

}
