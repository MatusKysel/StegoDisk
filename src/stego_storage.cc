//
//  StegoStorage.cpp
//  StegoDiskCore
//
//  Created by Martin Kosdy on 15/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//
#include "stego_storage.h"

#include <fstream>

#include "virtual_storage/virtual_storage.h"
#include "file_management/carrier_files_manager.h"
#include "utils/stego_config.h"
#include "utils/json.h"

using namespace std;

namespace stego_disk {

StegoStorage::StegoStorage() :
  carrier_files_manager_(new CarrierFilesManager()), opened_(false) {}

StegoStorage::~StegoStorage() {}

void StegoStorage::Open(const std::string &storage_base_path,
                        const std::string &password) {

  if(!StegoConfig::initialized()) {
    throw std::runtime_error("Storage must be configured before opening");
  }
  opened_ = false;

  carrier_files_manager_->SetPassword(password);
  carrier_files_manager_->LoadDirectory(storage_base_path);

  opened_ = true;
}

void StegoStorage::Configure(const std::string &config_path) const {
  std::ifstream ifs(config_path.c_str());
  if (!ifs.is_open()) {
    //std::cout << "Failed to open config file: " << config_path << std::endl; TODO(MATUS) log it
  }

  std::string json_string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  ifs.close();

  json::JsonObject config;
  std::string parse_error = json::Parse(json_string, &config);
  if (!parse_error.empty()) {
    //      std::cout << "Failed to parse config file: " << parse_error << std::endl; TODO(Matus) log it
  }

}

void StegoStorage::Configure() const {
  Configure(carrier_files_manager_->GetPath() + CONFIG_NAME);
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
  if (!opened_)
    throw std::runtime_error("Storage must be opened_ before loading");

  carrier_files_manager_->SetEncoder(
        EncoderFactory::GetEncoder(StegoConfig::encoder()));
  carrier_files_manager_->ApplyEncoder();

  virtual_storage_ = make_shared<VirtualStorage>();
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

void StegoStorage::Read(void* destination, std::size_t offSet,
                        std::size_t length) const {
  if (virtual_storage_ == nullptr)
    throw std::runtime_error("Storage must be loaded before use");

  virtual_storage_->Read(offSet, length, (uint8*) destination);
}

void StegoStorage::Write(void* source, std::size_t offSet, std::size_t length) {
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
