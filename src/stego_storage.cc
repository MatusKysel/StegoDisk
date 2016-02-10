//
//  StegoStorage.cpp
//  StegoDiskCore
//
//  Created by Martin Kosdy on 15/04/14.
//  Copyright (c) 2014 Martin Kosdy. All rights reserved.
//
#include "stego_storage.h"

#include "virtual_storage/virtual_storage.h"
#include "file_management/carrier_files_manager.h"

using namespace std;

namespace stego_disk {

StegoStorage::StegoStorage() :
  carrier_files_manager_(new CarrierFilesManager()), opened_(false) {}

StegoStorage::~StegoStorage() {}

void StegoStorage::Open(std::string storage_base_path, std::string password) {
  opened_ = false;

  carrier_files_manager_->SetPassword(password);
  carrier_files_manager_->LoadDirectory(storage_base_path);

  opened_ = true;
}

void StegoStorage::Load(EncoderFactory::EncoderType encoder,
                        PermutationFactory::PermutationType global_perm,
                        PermutationFactory::PermutationType local_perm) {
  if (!opened_)
    throw std::runtime_error("Storage must be opened_ before loading");

  carrier_files_manager_->SetEncoder(
        EncoderFactory::GetEncoder(encoder));
  carrier_files_manager_->ApplyEncoder();

  virtual_storage_ = make_shared<VirtualStorage>();
  virtual_storage_->SetPermutation(
        PermutationFactory::GetPermutation(global_perm));
  carrier_files_manager_->LoadVirtualStorage(virtual_storage_);
}

void StegoStorage::Load() {
  if (!opened_)
    throw std::runtime_error("Storage must be opened_ before loading");

  carrier_files_manager_->SetEncoder(
        EncoderFactory::GetDefaultEncoder());
  carrier_files_manager_->ApplyEncoder();

  virtual_storage_ = make_shared<VirtualStorage>();
  virtual_storage_->SetPermutation(
        PermutationFactory::GetDefaultPermutation());
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
