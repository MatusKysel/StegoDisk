//
//  CarrierFile.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 1/6/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include "Utils/StegoHeader.h"
#include "CarrierFiles/CarrierFile.h"
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include "Utils/Keccak/keccak.h"
#include "Utils/StegoErrors.h"
#include "Utils/Config.h"
#include "Permutations/PermutationFactory.h"
#include "Encoders/Encoder.h"

namespace StegoDisk {

CarrierFile::CarrierFile(File file, std::shared_ptr<Encoder> encoder)
    : _rawCapacity(0),
      _capacity(0),
      _dataBlockSize(0),
      _blocksUsed(0),
      _blockCount(0),
      _file(file),
      _fileLoaded(false),
	  _codewordBlockSize(0),
      _encoder(encoder)
{
    _virtualStorage = VirtualStoragePtr(nullptr);
    _virtualStorageOffset = 0;

    _permutation = PermutationFactory::getDefaultPermutation();

    if (encoder)
    {
        _dataBlockSize = encoder->getDataBlockSize();
        _codewordBlockSize = encoder->getCodewordBlockSize();
    }
}

CarrierFile::~CarrierFile()
{
}

File CarrierFile::getFile()
{
    return _file;
}

void CarrierFile::unsetEncoder()
{
    setEncoder(std::shared_ptr<Encoder>(nullptr));
}

void CarrierFile::setEncoder(std::shared_ptr<Encoder> encoder)
{
    if (!encoder) {
        _dataBlockSize = 1;
        _codewordBlockSize = 1;
        _blockCount = 0;
        _capacity = 0;
        _encoder = std::shared_ptr<Encoder>(nullptr);
    } else {
        _encoder = encoder;
        _dataBlockSize = encoder->getDataBlockSize();
        _codewordBlockSize = encoder->getCodewordBlockSize();
        //_blockCount = (_rawCapacity / _embedder->getCodewordBlockSize());
        _blockCount = (uint32)((_permutation->getSizeUsingParams(_rawCapacity*8, subkey) / 8) / encoder->getCodewordBlockSize());
        _capacity = _blockCount * encoder->getDataBlockSize();
    }
}

uint64 CarrierFile::getCapacityUsingEncoder(std::shared_ptr<Encoder> encoder)
{
    if (!encoder) return 0;
    if (!_permutation) return 0;
    uint64 blockCount = ((_permutation->getSizeUsingParams(_rawCapacity*8, subkey) / 8) / encoder->getCodewordBlockSize());
    return (blockCount * encoder->getDataBlockSize());
}

uint64 CarrierFile::getCapacity()
{
    return _capacity;
}

uint32 CarrierFile::getBlockCount()
{
    return _blockCount;
}

Key CarrierFile::getPermKey()
{
    //TODO: hashovat sa nesmie cesta ale iba nazov suboru! resp relativna cesta
    
    string buf = "";
    char timeStrBuf[40];
    buf.append(File::normalizePath(_file.getRelativePath()));
#ifdef STEGO_OS_WIN
	sprintf_s(timeStrBuf, sizeof(timeStrBuf), "%ld", _stat.st_mtime);
#else
	sprintf(timeStrBuf, "%ld", _stat.st_mtime);
#endif
    
    //buf.append(timeStrBuf);
	LOG_TRACE("CarrierFile::getPermKeyHash: hashing file attributes: " << buf);
    
    return Key::fromString(buf);
}

bool CarrierFile::isFileLoaded()
{
    return _fileLoaded;
}

int CarrierFile::addToVirtualStorage(VirtualStoragePtr storage, uint64 offset, uint64 bytesUsed)
{
    _virtualStorage = storage;
    _virtualStorageOffset = offset;

    if (bytesUsed) {
        _blocksUsed = (uint32)((bytesUsed-1) / _dataBlockSize) + 1;
    } else {
        _blocksUsed = 0;
    }

    return 0;
}

bool CarrierFile::operator< (const CarrierFile& val) const {

    string strA = _file.getRelativePath();
    string strB = val._file.getRelativePath();

    std::transform(strA.begin(), strA.end(), strA.begin(), ::tolower);
    std::transform(strB.begin(), strB.end(), strB.begin(), ::tolower);

    //LOG_DEBUG("operator< was called for string: " << _relativePath << " vs " << val._relativePath << ", transformed: " << strA << " vs " << strB);

    return (strA.compare(strB) < 0);
}

bool CarrierFile::compareByPointers(CarrierFile* a, CarrierFile* b)
{
    if (a == nullptr)
        return 0;
    
    
    if (b == nullptr)
        return 0;
    
    return (*a < *b);
}

bool CarrierFile::compareBySharedPointers(std::shared_ptr<CarrierFile> a, std::shared_ptr<CarrierFile> b)
{
    if (a == nullptr)
        return 0;
    
    
    if (b == nullptr)
        return 0;
    
    return (*(a.get()) < *(b.get()));
}

    
uint64 CarrierFile::getRawCapacity()
{
    return _rawCapacity;
}

void CarrierFile::setSubkey(const Key& subkey)
{
    this->subkey = subkey;
}

void CarrierFile::setBitInBufferPermuted(uint64 index)
{
	// TODO: rewrite to run-time exception
    if (index >= _permutation->getSize()) {
        LOG_INFO("CarrierFile::setBitInBufferPermuted: index " << index << " is too big!");
        return;
    }

    uint64 permutedIndex = _permutation->permute(index);

	// TODO: this check only in debug mode?
#ifdef _DEBUG
	if (permutedIndex >= _permutation->getSize()) {
        LOG_INFO("CarrierFile::setBitInBufferPermuted: permuted index " << permutedIndex << " is too big!");
        return;
    }
#endif

    _buffer[permutedIndex/8] |= (1 << (permutedIndex % 8));
}

uint8 CarrierFile::getBitInBufferPermuted(uint64 index)
{

    if (index >= _permutation->getSize()) {
        LOG_INFO("CarrierFile::getBitInBufferPermuted: index " << index << " is too big!");
        return 0;
    }

    uint64 permutedIndex = _permutation->permute(index);

    return ((_buffer[permutedIndex/8] & (1 << (permutedIndex % 8))) != 0);
}

int CarrierFile::extractBufferUsingEncoder()
{
    if (!_buffer.getSize()) return -1;
    if (!_encoder) return -2;
    if (!_codewordBlockSize) return -3;
    if (!_blocksUsed) return -4;

   MemoryBuffer dataBuffer(_dataBlockSize);

    //for (uint64 b=0;b<_blockCount;b++) {
    for (uint64 b=0;b<_blocksUsed;b++) {
        _encoder->extract(&_buffer[b*_codewordBlockSize], dataBuffer.getRawPointer());

        for (uint64 i=0;i<_dataBlockSize;i++) {
//TODO:            #warning doriesit try catch!
            try {
                _virtualStorage->writeByte(_virtualStorageOffset+(b*_dataBlockSize)+i, dataBuffer[i]);
            } catch (std::out_of_range& ex) {
				LOG_TRACE("CarrierFile::extractBufferUsingEncoder: virtualStorage->writeByte failed: block: " << (b+1) << "/" << _blocksUsed << ", byte: " << (i+1) << "/" << _dataBlockSize);
                //TODO: poriesit
				//LOG_TRACE("CarrierFile::extractBufferUsingEncoder: virtualStorage->writeByte failed: error code: " << errc << ", block: " << (b+1) << "/" << _blocksUsed << ", byte: " << (i+1) << "/" << _dataBlockSize);
                // TODO: poriesit ako zistit kedy je to error a kedy koniec uloziska -> to je ok
				// TODO: _virtualStorage->_rawCapacity testovat
                //LOG_ERROR("ERROR WRITING BYTE! error code: " << errc << ", block: " << (b+1) << "/" << _blocksUsed << ", byte: " << (i+1) << "/" << _dataBlockSize);
            }
        }
    }

    return 0;
}


int CarrierFile::embedBufferUsingEncoder()
{
    //TODO: rewrite using exceptions!
    if (_buffer.getSize() == 0) return -1;
    if (!_encoder) return -2;
    if (!_codewordBlockSize) return -3;
    if (!_blocksUsed) return -4;

    MemoryBuffer dataBuffer(_dataBlockSize);

    //for (uint64 b=0;b<_blockCount;b++) {
    for (uint64 b=0;b<_blocksUsed;b++) {
        for (uint64 i=0;i<_dataBlockSize;i++) {
//TODO:            #warning doriesit try catch!
            try {
                dataBuffer[i] = _virtualStorage->readByte(_virtualStorageOffset+(b*_dataBlockSize)+i);
            } catch (std::out_of_range& ex) {
				LOG_TRACE("CarrierFile::embedBufferUsingEncoder: virtualStorage->readByte failed: block: " << (b+1) << "/" << _blocksUsed << ", byte: " << (i+1) << "/" << _dataBlockSize);
                //TODO: poriesit ako zistit kedy je to error a kedy koniec uloziska -> to je ok
                //LOG_ERROR("ERROR READING BYTE! error code: " << errc << ", block: " << (b+1) << "/" << _blocksUsed << ", byte: " << (i+1) << "/" << _dataBlockSize);
                dataBuffer[i] = 0; // TODO: not sure if random data would be better here
            }
        }
        _encoder->embed(&_buffer[b*_codewordBlockSize], dataBuffer.getRawPointer());
    }

    return 0;
}

} // namespace StegoDisk
