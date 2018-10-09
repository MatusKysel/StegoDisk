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

#include "api_mask.h"
#include "file_management/carrier_files_manager.h"
#include "utils/exceptions.h"
#include "utils/stego_config.h"
#include "virtual_storage/virtual_storage.h"

namespace stego_disk {

StegoStorage::StegoStorage()
{
	carrier_files_manager_ = std::make_unique<CarrierFilesManager>();
}

StegoStorage::~StegoStorage() {}

void StegoStorage::Open(const std::string &storage_base_path,
                        const std::string &password,
						const std::string &filter) {
  opened_ = false;
  auto used_filter = filter;

  if (used_filter == "")
  {
	  used_filter = carrier_files_manager_->CreateFilterFromConfig();
  }

  carrier_files_manager_->SetPassword(password);
  carrier_files_manager_->LoadDirectory(storage_base_path, used_filter);

  opened_ = true;
}

void StegoStorage::Configure(const std::string &config_path) const {
  std::ifstream ifs(config_path.c_str());
  if (!ifs.is_open()) {
    throw exception::ErrorOpenFIle{config_path};
  }

  std::string json_string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  ifs.close();

  json::JsonObject config;
  std::string parse_error = json::Parse(json_string, &config);

  if (!parse_error.empty()) {
    throw exception::ParseError{config_path, parse_error};
  }
  StegoConfig::Init(config);
}

void StegoStorage::Configure() const {
  Configure(EncoderFactory::GetDefaultEncoderType(),
            PermutationFactory::GetDefaultPermutationType(),
            PermutationFactory::GetDefaultPermutationType());
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
    throw exception::InvalidState{exception::Operation::loadStegoStrorage,
                                  exception::Component::storage,
								  exception::ComponentState::notConfigured};
  }
  if (!opened_) {
    throw exception::InvalidState{exception::Operation::loadStegoStrorage,
                                  exception::Component::storage,
								  exception::ComponentState::notOpened};
  }
  
  try {
    carrier_files_manager_->SetEncoder(
          EncoderFactory::GetEncoder(StegoConfig::encoder()));
    carrier_files_manager_->ApplyEncoder();

    virtual_storage_ = std::make_shared<VirtualStorage>();
    virtual_storage_->SetPermutation(
          PermutationFactory::GetPermutation(StegoConfig::global_perm()));
    carrier_files_manager_->LoadVirtualStorage(virtual_storage_);
  }
  catch (...) { throw; }
}

void StegoStorage::Save() {
  if (!opened_)
    throw exception::InvalidState{exception::Operation::saveStegoStrorage,
                                  exception::Component::storage,
								  exception::ComponentState::notOpened};

  if (virtual_storage_ == nullptr)
    throw exception::InvalidState{exception::Operation::saveStegoStrorage,
                                  exception::Component::storage,
								  exception::ComponentState::notLoaded};

  try {
    carrier_files_manager_->SaveVirtualStorage();
  }
  catch (...) { throw; }
}

void StegoStorage::Read(void* destination, const std::size_t offSet,
                        const std::size_t length) const {
  if (virtual_storage_ == nullptr)
    throw exception::InvalidState{exception::Operation::ioStegoStorage,
                                  exception::Component::storage,
								  exception::ComponentState::notLoaded};

  try {
    virtual_storage_->Read(offSet, length, (uint8*)destination);
  }
  catch (...) { throw; }
}

void StegoStorage::Write(const void* source, const std::size_t offSet,
                         const std::size_t length) const {
  if (virtual_storage_ == nullptr)
    throw exception::InvalidState{exception::Operation::ioStegoStorage,
                                  exception::Component::storage,
								  exception::ComponentState::notLoaded};

  try {
    virtual_storage_->Write(offSet, length, (uint8*)source);
  }
  catch (...) { throw; }
}

std::size_t StegoStorage::GetSize() const {
  if (virtual_storage_ == nullptr)
    return 0;

  try {
    return virtual_storage_->GetUsableCapacity();
  }
  catch (...) { throw; }
}

void StegoStorage::ChangeEncoder(std::string &config) const {

  json::JsonObject json_config;
  std::string parse_error = json::Parse(config, &json_config);

  if (!parse_error.empty()) {
    throw exception::ParseError{config, parse_error};
  }

  try {
//    return carrier_files_manager_->GetCapacityUsingEncoder(encoder);
  }
  catch (...) { throw; }
}

} // stego_disk
