//
//  CarrierFilesManager.h
//  StegoFS
//
//  Created by Martin Kosdy on 1/6/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef __StegoFS__CarrierFilesManager__
#define __StegoFS__CarrierFilesManager__

#include <iostream>
#include <vector>
#include <string>
#include <memory>
//#include "FileManagement/CarrierFilesManagerDelegate.h"
#include "Hash/Hash.h"
#include "Keys/Key.h"

namespace StegoDisk {

class Encoder;
class VirtualStorage;
class CarrierFile;

class CarrierFilesManager {

public:
    CarrierFilesManager();
    ~CarrierFilesManager();
    int loadDirectory(std::string directory);
    void saveAllFiles(); 
       
    uint64 getCapacity();
    uint64 getRawCapacity();

    uint64 getCapacityUsingEncoder(std::shared_ptr<Encoder> encoder);

    void applyEncoder();
    void setEncoder(std::shared_ptr<Encoder> encoder);
    void unsetEncoder();
    void setEncoderArgByName(const std::string &param, const std::string &val);

    void setPassword(std::string password);

    bool loadVirtualStorage(std::shared_ptr<VirtualStorage> storage);
    int saveVirtualStorage();

//    void setDelegate(CarrierFilesManagerDelegate *delegate);


private:
    void init();
    
    void addFileAtPath(std::string path);

    void generateMasterKey();
    void deriveSubkeys();

    std::string _basePath;

    std::vector<std::shared_ptr<CarrierFile>> _carrierFiles;
    uint64 _capacity;

    uint64 _filesInDirectory;
    uint64 _loadingProgress;

    Hash passwordHash;
    Key masterKey;

//    CarrierFilesManagerDelegate* _delegate;
    std::shared_ptr<VirtualStorage> _virtualStorage;

    // TODO using of encoder in doc
    // 1. setEncoder (isActiveEnc = false)
    // 2. setEncoderArgs (isActiveEnc = false)
    // 3. applyEncoder (only after applyEnc isActiveEnc = true)
    // 4. ...
    // 5. unsetEncoder (isActiveEnc = false)
    std::shared_ptr<Encoder> _encoder;
    bool _isActiveEncoder;

//public slots:
//    void on_fileInitFinished();
//    void on_fileLoadFinished();
};

}

#endif /* defined(__StegoFS__CarrierFilesManager__) */
