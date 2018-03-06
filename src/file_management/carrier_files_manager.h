/**
* @file carrier_files_manager.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Class for managing carrier files
*
*/

#ifndef STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGER_H_
#define STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGER_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

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
  void LoadDirectory(const std::string &directory);
  void SaveAllFiles();

  uint64 GetCapacity();
  uint64 GetRawCapacity();
  std::string GetPath() const;
  uint64 GetCapacityUsingEncoder(std::shared_ptr<Encoder> encoder);

  void ApplyEncoder();
  void SetEncoder(std::shared_ptr<Encoder> encoder);
  void UnSetEncoder();
  void SetEncoderArg(const std::string &param, const std::string &val);

  void SetPassword(const std::string &password);

  bool LoadVirtualStorage(std::shared_ptr<VirtualStorage> storage);
  void SaveVirtualStorage();

private:
  void Init();

  void AddFileAtPath(std::string &path);

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
  std::unique_ptr<ThreadPool> thread_pool_;
  bool is_active_encoder_;
};

} // stego_disk

#endif // STEGODISK_FILEMANAGEMENT_CARRIERFILESMANAGER_H_
