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
#include "utils/thread_pool.h"

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

private:
  void Init();

  void AddFileAtPath(std::string path);

  void GenerateMasterKey();
  void DeriveSubkeys();

  std::string base_path_;

  std::vector<std::shared_ptr<CarrierFile>> carrier_files_;
  uint64 capacity_;

  uint64 files_in_directory_;

  Hash password_hash_;
  Key master_key_;

  std::shared_ptr<VirtualStorage> virtual_storage_;
  std::shared_ptr<Encoder> encoder_;
  bool is_active_encoder_;

  std::unique_ptr<ThreadPool> thread_pool_;

};

} // stego_disk

#endif // STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGER_H_
