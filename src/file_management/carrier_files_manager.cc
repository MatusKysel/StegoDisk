//
//  CarrierFilesManager.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 1/6/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//

#include "carrier_files_manager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
//#include <dirent.h>
//#include <QtCore>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "carrier_files/carrier_file_factory.h"
//#include "carrier_files/carrier_file_bmp.h"
#include "carrier_files/carrier_file_jpeg.h"
#include "utils/keccak/keccak.h"
#include "utils/stego_errors.h"
#include "utils/config.h"
#include "utils/stego_math.h"
#include "utils/file.h"
#include "utils/cpp11_compat.h"
#include "logging/logger.h"
//#include <QThreadPool>
//#include <unistd.h>

using namespace std;

namespace stego_disk {

CarrierFilesManager::CarrierFilesManager() :
  capacity_(0),
  files_in_directory_(0),
  loading_progress_(0),
  virtual_storage_(VirtualStoragePtr(nullptr)),
  encoder_(std::shared_ptr<Encoder>(nullptr)),
  is_active_encoder_(false) {}

CarrierFilesManager::~CarrierFilesManager() {
  carrier_files_.clear();
}

//void CarrierFilesManager::on_fileInitFinished()
//{
//    int loadingProgress;
//    loading_progress_Mutex.lock();
//    loadingProgress = ++loading_progress_;
//    loading_progress_Mutex.unlock();

//    if (_delegate) {
//        _delegate->cfmProgressUpdated(loadingProgress, files_in_directory_, CountingCapacity);
//    }
//}


//void CarrierFilesManager::on_fileLoadFinished()
//{
//    int loadingProgress;
//    loading_progress_Mutex.lock();
//    loadingProgress = ++loading_progress_;
//    loading_progress_Mutex.unlock();

//    if (_delegate) {
//        _delegate->cfmProgressUpdated(loadingProgress, carrier_files_.size(), LoadingFiles);
//    }
//}


int CarrierFilesManager::LoadDirectory(std::string directory) {
  //    _loadingThreadPool.waitForDone();

  carrier_files_.clear();
  capacity_ = 0;
  files_in_directory_ = 0;

  base_path_ = directory;

  vector<File> files = File::GetFilesInDir(directory, "");

  files_in_directory_ = files.size();



  //LOG_TRACE("CarrierFilesManager::loadDirectory: dir enumeration results begin:");
  FOREACH( File, files, file ) {
    LOG_TRACE(FOREACH_ELM(file).GetBasePath() + " - " +
              FOREACH_ELM(file).GetRelativePath());
  }
  //LOG_TRACE("CarrierFilesManager::loadDirectory: dir enumeration results end");



  /*
    CarrierFileInitTask *task = new CarrierFileInitTask(base_path_, QDir(base_path_).relativeFilePath(sFilePath), &carrier_files_, &_fileListMutex);
    QObject::connect(task, SIGNAL(finished()), this, SLOT(on_fileInitFinished()));
    _loadingThreadPool.start(task);
*/
  FOREACH( File, files, file ) {
    CarrierFilePtr carrier_file =
        CarrierFileFactory::CreateCarrierFile(FOREACH_ELM(file));
    if (carrier_file != nullptr) {
      carrier_files_.push_back(carrier_file);
    }
  }

  for (uint64 i = 0; i < carrier_files_.size(); ++i) {
    LOG_TRACE("CarrierFilesManager::loadDirectory: '" <<
              carrier_files_[i]->GetFile().GetRelativePath() <<
              "' has raw capacity " << carrier_files_[i]->GetRawCapacity());
  }

  std::sort(carrier_files_.begin(), carrier_files_.end(),
            CarrierFile::CompareBySharedPointers);

  return STEGO_NO_ERROR;
}

// return false, if checksum is not valid, true otherwise
// TODO mY check PERMUTATION init by PASSWORD
bool CarrierFilesManager::LoadVirtualStorage(VirtualStoragePtr storage) {
  if (!storage)
    throw std::invalid_argument("CarrierFilesManager::loadVirtualStorage: "
                                "arg 'storage' is nullptr");
  if (!encoder_)
    throw std::invalid_argument("CarrierFilesManager::loadVirtualStorage: "
                                "encoder is not Set yet");
  if (!is_active_encoder_)
    throw std::invalid_argument("CarrierFilesManager::loadVirtualStorage: "
                                "encoder is not applied yet");

  // mY moved to CFM::applyEnc, we need call CF::SetSubkey before CF::SetEnc
  //    generateMasterKey();
  //    deriveSubkeys();

  try { storage->ApplyPermutation(this->GetCapacity(), master_key_); }
  catch (...) { throw; }

  uint64 offset = 0;

  uint64 remaining_capacity = storage->GetRawCapacity();

  uint64 bytes_used;

  //_loadingThreadPool.waitForDone();
  loading_progress_ = 0;

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

    /*
        if (_progressCallback != NULL) {
            _progressCallback(i+1, carrier_files_.size(), LoadingFiles);
        }
        */
    /*
        if (_delegate) {
            _delegate->progressUpdated(i+1, carrier_files_.size(), LoadingFiles);
        }
        */
  }

  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    carrier_files_[i]->LoadFile();
    //        CarrierFileLoadTask *task = new CarrierFileLoadTask(carrier_files_[i]);
    //        QObject::connect(task, SIGNAL(finished()), this, SLOT(on_fileLoadFinished()));
    //        _loadingThreadPool.start(task);
  }

  //    _loadingThreadPool.waitForDone();

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

int CarrierFilesManager::SaveVirtualStorage() {
  if (!virtual_storage_) return SE_UNINITIALIZED;

  virtual_storage_->WriteChecksum();

  SaveAllFiles();

  return STEGO_NO_ERROR;
}

void CarrierFilesManager::SetEncoderArgByName(const string &param,
                                              const string &val) {
  if (!encoder_)
    throw std::invalid_argument("CarrierFilesManager::SetEncoderArgByName: "
                                "encoder is not Set yet");
  if (is_active_encoder_)
    throw std::invalid_argument("CarrierFilesManager::SetEncoderArgByName: "
                                "another encoder is active; please first unSet "
                                "this encoder");

  try { EncoderFactory::SetEncoderArgByName(encoder_,param,val); }
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
    throw std::invalid_argument("CarrierFilesManager::SetEncoder: "
                                "input arg 'encoder' is nullptr");
  if (encoder_)
    UnSetEncoder();
  encoder_ = encoder;
  is_active_encoder_ = false;
}

void CarrierFilesManager::ApplyEncoder() {
  if (!encoder_)
    throw std::invalid_argument("CarrierFilesManager::applyEncoder: encoder is not Set yet");

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
    throw std::out_of_range("CarrierFilesManager::applyEncoder: "
                            "not enough stego space in carrier files");

  capacity_ = capacity;
  is_active_encoder_ = true;
  LOG_DEBUG("CarrierFilesManager::applyEncoder: all carrier files sum: raw_cap="
            << raw_cap << ", cap=" << capacity);
}


void CarrierFilesManager::SetPassword(std::string password) {
  password_hash_.Process(password);
  LOG_DEBUG("CarrierFilesManager::SetPassword: Setting password: '"
            << password << "'");
}


/**
 * @brief Generates Master Key hash from password (hash) and keys generated from individual carrier files
 * @return Error code (0 = NO ERROR)
 */
void CarrierFilesManager::GenerateMasterKey() {
  if (carrier_files_.size() < 1) {
    LOG_ERROR("Nothing to hash, no files loaded...");
    //TODO: throw exception?
    return;
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
  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    carrier_files_[i]->SaveFile();
    //        if (_delegate) {
    ////            _delegate->cfmProgressUpdated(i+1, carrier_files_.size(), SavingFiles);
    //        }
  }
}


uint64 CarrierFilesManager::GetCapacity() {
  if (!encoder_)
    throw std::invalid_argument("CarrierFilesManager::GetCapacity: "
                                "encoder is not Set yet");
  if (!is_active_encoder_)
    throw std::invalid_argument("CarrierFilesManager::GetCapacity: "
                                "encoder is not applied yet");
  return capacity_;
}

uint64 CarrierFilesManager::GetRawCapacity() {
  uint64 capacity = 0;
  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    capacity += carrier_files_.at(i)->GetRawCapacity();
  }
  return capacity;
}

/*
void CarrierFilesManager::SetProgressCallback(ProgressCallbackFunction progressCallback)
{
    _progressCallback = progressCallback;
}
*/

//void CarrierFilesManager::SetDelegate(CarrierFilesManagerDelegate *delegate)
//{
//    _delegate = delegate;
//}


uint64 CarrierFilesManager::GetCapacityUsingEncoder(
    std::shared_ptr<Encoder> encoder) {
  if (!encoder)
    throw std::invalid_argument("CarrierFilesManager::GetCapacity: arg "
                                "'encoder' is nullptr");
  uint64 capacity = 0;
  for (size_t i = 0; i < carrier_files_.size(); ++i) {
    capacity += carrier_files_.at(i)->GetCapacityUsingEncoder(encoder);
  }
  return capacity;
}

} // stego_disk
