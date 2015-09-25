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
#include "encoders/encoder_factory.h"

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

void StegoStorage::Load() {
  if (!opened_)
    throw std::runtime_error("Storage must be opened_ before loading");

  // Set encoder
  // TODO manager->SetPermutation for carrier files
  carrier_files_manager_->SetEncoder(
        EncoderFactory::GetEncoderByName("hamming"));
  //carrier_files_manager_.SetEncoder(EncoderFactory::GetEncoderByName("lsb"));
  //carrier_files_manager_.SetEncoderArgByName("blockSize","1");
  carrier_files_manager_->ApplyEncoder();

  virtual_storage_ = make_shared<VirtualStorage>();
  virtual_storage_->SetPermutation(
        PermutationFactory::GetPermutationByName("MixedFeistel"));
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
