//
//  CarrierFile.h
//  StegoFS
//
//  Created by Martin Kosdy on 1/6/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__CarrierFile__
#define __StegoFS__CarrierFile__

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include <typeinfo>
#include "Utils/Config.h"
#include "Utils/StegoTypes.h"
#include "Logging/logger.h"
#include "Encoders/EncoderFactory.h"
#include "Permutations/PermutationFactory.h"
#include "Permutations/Permutation.h"
#include "VirtualStorage/VirtualStorage.h"
#include "Utils/File.h"
#include "Utils/MemoryBuffer.h"
#include "Keys/Key.h"

using namespace std;

namespace StegoDisk {

/**
 * The CarrierFile class.
 *
 * Abstract base class CarrierFile (cannot be instantiated)
 * Provides an interface for CarrierFiles subclasses
 */
class CarrierFile {
        
public:
    CarrierFile(File file, std::shared_ptr<Encoder> encoder);
    virtual ~CarrierFile();
    
    uint64 getCapacity();
    uint64 getRawCapacity();
    uint32 getBlockCount();

    File getFile();

    void setPermutation(PermutationPtr permutation);
    void unsetPermutation();
    void setEncoder(std::shared_ptr<Encoder> encoder);
    void unsetEncoder();
    uint64 getCapacityUsingEncoder(std::shared_ptr<Encoder> encoder);

    virtual bool isFileLoaded();
    virtual int loadFile() = 0;
    virtual int saveFile() = 0;

    void setSubkey(const Key& subkey);
    int addToVirtualStorage(VirtualStoragePtr storage, uint64 offset, uint64 bytesUsed);
        
    Key getPermKey();
    
    bool operator< (const CarrierFile& val) const;

    static bool compareByPointers(CarrierFile* a, CarrierFile* b);
    static bool compareBySharedPointers(std::shared_ptr<CarrierFile> a, std::shared_ptr<CarrierFile> b);
    
protected:
    int setDatesBack();
    
    void setBitInBufferPermuted(uint64 index);
    uint8 getBitInBufferPermuted(uint64 index);

    int extractBufferUsingEncoder();
    int embedBufferUsingEncoder();

    MemoryBuffer _buffer;

    uint32 _codewordBlockSize;
    uint32 _dataBlockSize;
    uint32 _blockCount;
    uint64 _capacity;
    uint64 _rawCapacity;

    uint32 _blocksUsed;

    uint64 _virtualStorageOffset;

    bool _fileLoaded;
    struct stat _stat;

    Key subkey;

    File _file;
    std::shared_ptr<Encoder> _encoder;
    PermutationPtr _permutation;
    VirtualStoragePtr _virtualStorage;
};

} // namespace StegoDisk

#endif /* defined(__StegoFS__CarrierFile__) */
