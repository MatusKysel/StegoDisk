/**
* @file carrier_files_manager.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Class for managing carrier files
*
*/

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>

#include "api_mask.h"
#include "hash/hash.h"
#include "keys/key.h"
#include "utils/thread_pool.h"

namespace stego_disk {

class Encoder;
class VirtualStorage;
class CarrierFile;

static std::unordered_set<std::string> SupportedFormats {
	"jpg", "bmp", "png", "mkv", "mp4"
};

class CarrierFilesManager {

public:
  CarrierFilesManager();
  ~CarrierFilesManager();
  void LoadDirectory(const std::string &directory, const std::string &filter = "");
  void SaveAllFiles();

  uint64 GetCapacity() const;
  uint64 GetRawCapacity() const;
  std::string GetPath() const;
  uint64 GetCapacityUsingEncoder(std::shared_ptr<Encoder> encoder) const;

  void ApplyEncoder();
  void SetEncoder(std::shared_ptr<Encoder> encoder);
  void UnSetEncoder();
  void SetEncoderArg(const std::string &param, const std::string &val);

  void SetPassword(const std::string &password);

  bool LoadVirtualStorage(std::shared_ptr<VirtualStorage> storage);
  void SaveVirtualStorage();

  std::string CreateFilterFromConfig() const;

private:
  void GenerateMasterKey();
  void DeriveSubkeys();

  std::string base_path_;

  std::vector<std::shared_ptr<CarrierFile>> carrier_files_;
  uint64 capacity_{ 0 };
  uint64 files_in_directory_{ 0 };

  Hash password_hash_;
  Key master_key_;

  std::shared_ptr<VirtualStorage> virtual_storage_{ nullptr };
  std::shared_ptr<Encoder> encoder_{ nullptr };
  std::unique_ptr<ThreadPool> thread_pool_{ nullptr };
  bool is_active_encoder_{ false };
};

} // stego_disk
