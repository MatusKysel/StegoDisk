/**
* @file carrier_files_manager.h
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief Implementation of class for managing carrier files
*
*/

#include "carrier_files_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "carrier_files/carrier_file.h"
#include "carrier_files/carrier_file_factory.h"
#include "logging/logger.h"
#include "utils/config.h"
#include "utils/exceptions.h"
#include "utils/file.h"
#include "utils/keccak/keccak.h"
#include "utils/stego_config.h"
#include "utils/stego_errors.h"
#include "utils/stego_math.h"
#include "utils/thread_pool.h"
#include "virtual_storage/virtual_storage.h"

using namespace std;

namespace stego_disk {

CarrierFilesManager::CarrierFilesManager() :
  capacity_(0),
  files_in_directory_(0),
  virtual_storage_(std::shared_ptr<VirtualStorage>(nullptr)),
  encoder_(std::shared_ptr<Encoder>(nullptr)),
  thread_pool_(std::make_unique<ThreadPool>(0)),
  is_active_encoder_(false) {}

CarrierFilesManager::~CarrierFilesManager() {
  carrier_files_.clear();
}

std::string CarrierFilesManager::GetPath() const {
  return base_path_;
}

void CarrierFilesManager::LoadDirectory(const std::string &directory) {

  carrier_files_.clear();
  capacity_ = 0;
  files_in_directory_ = 0;

  base_path_ = directory;

  vector<File> files = File::GetFilesInDir(directory, ""); //PSTODO neskodila by nejaka filtracia

  files_in_directory_ = files.size();

  for(auto &file: files) {
     LOG_TRACE(file.GetBasePath() + " - " + file.GetRelativePath());
  }

  std::vector<std::future<CarrierFilePtr>> carrier_files;

  for(auto &file: files) {
    if(StegoConfig::exclude_list().find(file.GetExtension()) == StegoConfig::exclude_list().end()) {
      carrier_files.emplace_back(thread_pool_->enqueue(
                                   &CarrierFileFactory::CreateCarrierFile,
                                   file));
    }
  }

  for(auto &&file: carrier_files) {
    auto result = file.get();
    if (result != nullptr) {
      carrier_files_.push_back(result);
    }
  }


  for (uint64 i = 0; i < carrier_files_.size(); ++i) {
    LOG_TRACE("CarrierFilesManager::loadDirectory: '" <<
              carrier_files_[i]->GetFile().GetRelativePath() <<
              "' has raw capacity " << carrier_files_[i]->GetRawCapacity());
  }

  std::sort(carrier_files_.begin(), carrier_files_.end(),
            CarrierFile::CompareBySharedPointers);
}

// return false, if checksum is not valid, true otherwise
// TODO mY check PERMUTATION init by PASSWORD
bool CarrierFilesManager::LoadVirtualStorage(std::shared_ptr<VirtualStorage> storage) {
  if (!storage)
    throw exception::InvalidState{exception::Operation::loadVirtualStorage,
                                  exception::Component::storage,
								  exception::ComponentState::notInitialized};
  if (!encoder_)
    throw exception::InvalidState(exception::Operation::loadVirtualStorage,
                                  exception::Component::encoder,
								  exception::ComponentState::notSetted);
  if (!is_active_encoder_)
    throw exception::InvalidState(exception::Operation::loadVirtualStorage,
                                  exception::Component::encoder,
								  exception::ComponentState::notActive);

  try { storage->ApplyPermutation(this->GetCapacity(), master_key_); }
  catch (...) { throw; }

  uint64 offset = 0;

  uint64 remaining_capacity = storage->GetRawCapacity();

  uint64 bytes_used;

  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    if (remaining_capacity > carrier_files_[i]->GetCapacity()) {
      remaining_capacity -= carrier_files_[i]->GetCapacity();
      bytes_used = carrier_files_[i]->GetCapacity();
    } else {
      bytes_used = remaining_capacity;
      remaining_capacity = 0;
    }
    carrier_files_[i]->AddToVirtualStorage(storage, offset, bytes_used);
    offset += carrier_files_[i]->GetCapacity();
  }

  std::vector<std::future<void>> load_results;

  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    load_results.emplace_back(thread_pool_->enqueue(&CarrierFile::LoadFile,
                                                    carrier_files_[i]));
  }

  for(auto &&result: load_results) {
    try {result.get();}
    catch (...) { throw; }
  }

  virtual_storage_ = storage;
  try {
    if ( virtual_storage_->IsValidChecksum() == false ) {
      LOG_DEBUG("Data integrity test: checksum is NOT valid");
      return false;
    }
  } catch (...) { throw; }
  LOG_DEBUG("Data integrity test: checksum is valid");

  return true;
}

void CarrierFilesManager::SaveVirtualStorage() {
  if (!virtual_storage_)
	  throw exception::InvalidState(exception::Operation::saveVirtualStorage,
			                        exception::Component::virtualStorage,
									exception::ComponentState::notInitialized);

  virtual_storage_->WriteChecksum();

  SaveAllFiles();
}

void CarrierFilesManager::SetEncoderArg(const string &param,
                                        const string &val) {
  if (!encoder_)
    throw exception::InvalidState(exception::Operation::setEncoderArg,
                                  exception::Component::encoder,
                                  exception::ComponentState::notSetted);
  if (is_active_encoder_)
    throw exception::InvalidState(exception::Operation::setEncoderArg,
                                  exception::Component::encoder,
                                  exception::ComponentState::isActive);

  try { EncoderFactory::SetEncoderArg(encoder_, param, val); }
  catch (...) { throw; }
}

void CarrierFilesManager::UnSetEncoder() {

  for (size_t i = 0; i < carrier_files_.size(); ++i)
    carrier_files_[i]->UnSetEncoder();

  encoder_ = std::shared_ptr<Encoder>(nullptr);
  is_active_encoder_ = false;
}

void CarrierFilesManager::SetEncoder(std::shared_ptr<Encoder> encoder) {
  if (!encoder)
    throw exception::NullptrArgument{"encoder"};
  if (encoder_)
    UnSetEncoder();
  encoder_ = encoder;
  is_active_encoder_ = false;
}

void CarrierFilesManager::ApplyEncoder() {
  if (!encoder_)
    throw exception::InvalidState(exception::Operation::applyEncoder,
                                  exception::Component::encoder,
                                  exception::ComponentState::notSetted);

  uint64 capacity = 0;
  uint64 raw_cap = 0;

  // mY from CFM::loadVS
  GenerateMasterKey();
  DeriveSubkeys();

  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    carrier_files_[i]->SetEncoder(encoder_);
    capacity += carrier_files_[i]->GetCapacity();
    raw_cap += carrier_files_[i]->GetRawCapacity();
    LOG_DEBUG("CarrierFilesManager::applyEncoder: file '" <<
              carrier_files_[i]->GetFile().GetRelativePath() <<
              "': raw=" << carrier_files_[i]->GetRawCapacity() <<
              ", cap=" << carrier_files_[i]->GetCapacity());
  }
  if (capacity == 0)
    throw exception::ZeroAllocatedSize{};

  capacity_ = capacity;
  is_active_encoder_ = true;
  LOG_DEBUG("CarrierFilesManager::applyEncoder: all carrier files sum: raw_cap="
            << raw_cap << ", cap=" << capacity);
}


void CarrierFilesManager::SetPassword(const std::string &password) {
  password_hash_.Process(password);
  LOG_DEBUG("CarrierFilesManager::SetPassword: Setting password: '"
            << password << "'");
}


/**
 * @brief Generates Master Key hash from password (hash) and keys generated from individual carrier files
 */
void CarrierFilesManager::GenerateMasterKey() {
  if (carrier_files_.size() < 1) {
    LOG_ERROR("Nothing to hash, no files loaded...");
	throw exception::EmptyMember{"carrier_files"};
  }

  LOG_DEBUG("CarrierFilesManager::generateMasterKey: PSWD HASH is "
            << StegoMath::HexBufferToStr(password_hash_.GetState()));

  Hash master_hey_hash(password_hash_);

  // add keys generated from params of individual carrier files
  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    master_hey_hash.Append(carrier_files_[i]->GetPermKey().GetData());
  }

  master_key_ = Key(master_hey_hash.GetState());
}

void CarrierFilesManager::DeriveSubkeys() {
  if (carrier_files_.size() < 1) {
    LOG_ERROR("There is no carrier loaded...");
    return;
  }

  LOG_DEBUG("CarrierFilesManager::deriveSubkeys: master key is "
            << StegoMath::HexBufferToStr(master_key_.GetData()));

  Hash hash;

  for (uint64 i = 0; i < carrier_files_.size(); ++i) {

    // subkey = hash ( hash(master_key | file_index) | hash(file_path) )

    hash.Process(master_key_.GetData());
    hash.Append(std::to_string(i));
    hash.Append(carrier_files_[i]->GetFile().GetNormalizedPath());

    carrier_files_[i]->SetSubkey(Key(hash.GetState()));

    LOG_DEBUG("CarrierFilesManager::deriveSubkeys: subkey for carrier '"
              << carrier_files_[i]->GetFile().GetAbsolutePath() << "' is " <<
              StegoMath::HexBufferToStr(hash.GetState()));
  }

}


void CarrierFilesManager::SaveAllFiles() {
  std::vector<std::future<void>> save_results;

  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    save_results.emplace_back(
          thread_pool_->enqueue(&CarrierFile::SaveFile, carrier_files_[i]));
  }

  for(auto &&result: save_results) {
    try {result.get();}
    catch (...) { throw; }
  }
}


uint64 CarrierFilesManager::GetCapacity() {
  if (!encoder_)
    throw exception::InvalidState(exception::Operation::getCapacity,
                                  exception::Component::encoder,
                                  exception::ComponentState::notSetted);
  if (!is_active_encoder_)
    throw exception::InvalidState(exception::Operation::getCapacity,
                                  exception::Component::encoder,
                                  exception::ComponentState::notActive);
  return capacity_;
}

uint64 CarrierFilesManager::GetRawCapacity() {
  uint64 capacity = 0;
  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    capacity += carrier_files_.at(i)->GetRawCapacity();
  }
  return capacity;
}

uint64 CarrierFilesManager::GetCapacityUsingEncoder(
    std::shared_ptr<Encoder> encoder) {
  if (!encoder)
    throw exception::InvalidState(exception::Operation::getCapacity,
                                  exception::Component::encoder,
                                  exception::ComponentState::notSetted);
  uint64 capacity = 0;
  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    capacity += carrier_files_.at(i)->GetCapacityUsingEncoder(encoder);
  }
  return capacity;
}

} // stego_disk
