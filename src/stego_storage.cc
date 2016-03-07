/**
* @file stego_storage.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File containing implementation of StegoStorage interface.
*
*/

#include "stego_storage.h"

#include <fstream>

#include "virtual_storage/virtual_storage.h"
#include "file_management/carrier_files_manager.h"
#include "utils/stego_config.h"
#include "utils/json.h"

namespace stego_disk {

StegoStorage::StegoStorage() :
  carrier_files_manager_(new CarrierFilesManager()), opened_(false) {}

StegoStorage::~StegoStorage() {}

void StegoStorage::Open(const std::string &storage_base_path,
                        const std::string &password) {
  opened_ = false;

  carrier_files_manager_->SetPassword(password);
  carrier_files_manager_->LoadDirectory(storage_base_path);

  opened_ = true;
}

void StegoStorage::Configure(const std::string &config_path) const {
  std::ifstream ifs(config_path.c_str());
  if (!ifs.is_open()) {
    throw std::runtime_error("Failed to open config file " + config_path);
  }

  std::string json_string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  ifs.close();

  json::JsonObject config;
  std::string parse_error = json::Parse(json_string, &config);

  if (!parse_error.empty()) {
    throw std::runtime_error("Failed to parse config file " + parse_error);
  }
}

void StegoStorage::Configure() const {
  if (opened_) {
    Configure(carrier_files_manager_->GetPath() + CONFIG_NAME);
  } else {
    Configure(EncoderFactory::GetDefaultEncoderType(),
              PermutationFactory::GetDefaultPermutationType(),
              PermutationFactory::GetDefaultPermutationType());
  }
}

void StegoStorage::Configure(const EncoderFactory::EncoderType encoder,
                             const PermutationFactory::PermutationType global_perm,
                             const PermutationFactory::PermutationType local_perm) const {

  json::JsonObject config;
  config.AddToObject("encoder", EncoderFactory::GetEncoderName(encoder));
  config.AddToObject("global_perm", PermutationFactory::GetPermutationName(global_perm));
  config.AddToObject("local_perm",  PermutationFactory::GetPermutationName(local_perm));
  StegoConfig::Init(config);
}

void StegoStorage::Load() {

  if (!StegoConfig::initialized()) {
    throw std::runtime_error("Storage must be configured before opening");
  }
  if (!opened_) {
    throw std::runtime_error("Storage must be opened_ before loading");
  }

  carrier_files_manager_->SetEncoder(
        EncoderFactory::GetEncoder(StegoConfig::encoder()));
  carrier_files_manager_->ApplyEncoder();

  virtual_storage_ = std::make_shared<VirtualStorage>();
  virtual_storage_->SetPermutation(
        PermutationFactory::GetPermutation(StegoConfig::global_perm()));
  carrier_files_manager_->LoadVirtualStorage(virtual_storage_);
}

void StegoStorage::Save() {
  if (!opened_)
    throw std::runtime_error("Storage must be opened_ before saving");

  if (virtual_storage_ == nullptr)
    throw std::runtime_error("Storage must be loaded before saving");

  carrier_files_manager_->SaveVirtualStorage();
}

void StegoStorage::Read(const void* destination, const std::size_t offSet,
                        const std::size_t length) const {
  if (virtual_storage_ == nullptr)
    throw std::runtime_error("Storage must be loaded before use");

  virtual_storage_->Read(offSet, length, (uint8*) destination);
}

void StegoStorage::Write(const void* source, const std::size_t offSet,
                         const std::size_t length) const {
  if (virtual_storage_ == nullptr)
    throw std::runtime_error("Storage must be loaded before use");

  virtual_storage_->Write(offSet, length, (uint8*) source);
}

std::size_t StegoStorage::GetSize() const {
  if (virtual_storage_ == nullptr)
    return 0;

  return virtual_storage_->GetUsableCapacity();
}

} // stego_disk
