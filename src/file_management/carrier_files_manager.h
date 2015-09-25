//
//  CarrierFilesManager.h
//  StegoFS
//
//  Created by Martin Kosdy on 1/6/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#ifndef STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGER_H_
#define STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGER_H_

#include <iostream>
#include <vector>
#include <string>
#include <memory>

//#include "carrier_files_manager_delegate"
#include "hash/hash.h"
#include "keys/key.h"

namespace stego_disk {

class Encoder;
class VirtualStorage;
class CarrierFile;

class CarrierFilesManager {

public:
  CarrierFilesManager();
  ~CarrierFilesManager();
  int LoadDirectory(std::string directory);
  void SaveAllFiles();

  uint64 GetCapacity();
  uint64 GetRawCapacity();

  uint64 GetCapacityUsingEncoder(std::shared_ptr<Encoder> encoder);

  void ApplyEncoder();
  void SetEncoder(std::shared_ptr<Encoder> encoder);
  void UnSetEncoder();
  void SetEncoderArgByName(const std::string &param, const std::string &val);

  void SetPassword(std::string password);

  bool LoadVirtualStorage(std::shared_ptr<VirtualStorage> storage);
  int SaveVirtualStorage();

  //    void SetDelegate(CarrierFilesManagerDelegate *delegate);


private:
  void Init();

  void AddFileAtPath(std::string path);

  void GenerateMasterKey();
  void DeriveSubkeys();

  std::string base_path_;

  std::vector<std::shared_ptr<CarrierFile>> carrier_files_;
  uint64 capacity_;

  uint64 files_in_directory_;
  uint64 loading_progress_;

  Hash password_hash_;
  Key master_key_;

  //    CarrierFilesManagerDelegate* _delegate;
  std::shared_ptr<VirtualStorage> virtual_storage_;

  // TODO using of encoder in doc
  // 1. SetEncoder (isActiveEnc = false)
  // 2. SetEncoderArgs (isActiveEnc = false)
  // 3. applyEncoder (only after applyEnc isActiveEnc = true)
  // 4. ...
  // 5. unSetEncoder (isActiveEnc = false)
  std::shared_ptr<Encoder> encoder_;
  bool is_active_encoder_;

  //public slots:
  //    void on_fileInitFinished();
  //    void on_fileLoadFinished();
};

} // stego_disk

#endif // STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGER_H_
