//
//  CarrierFile.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 1/6/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "carrier_file.h"

#include <sys/stat.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>

#include "utils/stego_header.h"
#include "utils/keccak/keccak.h"
#include "utils/stego_errors.h"
#include "utils/config.h"
#include "permutations/permutation_factory.h"
#include "encoders/encoder.h"

namespace stego_disk {

CarrierFile::CarrierFile(File file, std::shared_ptr<Encoder> encoder)
  : codeword_block_size_(0),
    data_block_size_(0),
    block_count_(0),
    capacity_(0),
    raw_capacity_(0),
    blocks_used_(0),
    virtual_storage_offset_(0),
    file_loaded_(false),
    file_(file),
    encoder_(encoder),
    permutation_(PermutationFactory::GetDefaultPermutation()),
    virtual_storage_(VirtualStoragePtr(nullptr)) {

  if (encoder)  {
    data_block_size_ = encoder->GetDataBlockSize();
    codeword_block_size_ = encoder->GetCodewordBlockSize();
  }
}

CarrierFile::~CarrierFile() {}

File CarrierFile::GetFile() {
  return file_;
}

void CarrierFile::UnSetEncoder() {
  SetEncoder(std::shared_ptr<Encoder>(nullptr));
}

void CarrierFile::SetEncoder(std::shared_ptr<Encoder> encoder) {
  if (!encoder) {
    data_block_size_ = 1;
    codeword_block_size_ = 1;
    block_count_ = 0;
    capacity_ = 0;
    encoder_ = std::shared_ptr<Encoder>(nullptr);
  } else {
    encoder_ = encoder;
    data_block_size_ = encoder->GetDataBlockSize();
    codeword_block_size_ = encoder->GetCodewordBlockSize();
    block_count_ = static_cast<uint32>((permutation_->GetSizeUsingParams(
                                          raw_capacity_ * 8, subkey_) / 8)
                                       / encoder->GetCodewordBlockSize());
    capacity_ = block_count_ * encoder->GetDataBlockSize();
  }
}

uint64 CarrierFile::GetCapacityUsingEncoder(std::shared_ptr<Encoder> encoder) {
  if (encoder && permutation_) return 0;
  uint64 block_count = ((permutation_->GetSizeUsingParams(
                           raw_capacity_ * 8, subkey_) / 8)
                        / encoder->GetCodewordBlockSize());
  return (block_count * encoder->GetDataBlockSize());
}

uint64 CarrierFile::GetCapacity() {
  return capacity_;
}

uint32 CarrierFile::GetBlockCount() {
  return block_count_;
}

Key CarrierFile::GetPermKey() {
  //TODO: hashovat sa nesmie cesta ale iba nazov suboru! resp relativna cesta

  string buf = "";
  //  char time_str_buf[40];
  buf.append(File::NormalizePath(file_.GetRelativePath()));
  //#ifdef STEGO_OS_WIN
  //  sprintf_s(time_str_buf, sizeof(time_str_buf), "%ld", stat_.st_mtime);
  //#else
  //  sprintf(time_str_buf, "%40ld", stat_.st_mtime);
  //#endif

  //buf.append(time_str_buf);
  LOG_TRACE("CarrierFile::GetPermKeyHash: hashing file attributes: " << buf);

  return Key::FromString(buf);
}

bool CarrierFile::IsFileLoaded() {
  return file_loaded_;
}

int CarrierFile::AddToVirtualStorage(VirtualStoragePtr storage, uint64 offset,
                                     uint64 bytes_used) {
  virtual_storage_ = storage;
  virtual_storage_offset_ = offset;

  if (bytes_used) {
    blocks_used_ = static_cast<uint32>(((bytes_used - 1) / data_block_size_) + 1);
  } else {
    blocks_used_ = 0;
  }

  return 0;
}

bool CarrierFile::operator< (const CarrierFile& val) const {

  string str_a = file_.GetRelativePath();
  string str_b = val.file_.GetRelativePath();

  std::transform(str_a.begin(), str_a.end(), str_a.begin(), ::tolower);
  std::transform(str_b.begin(), str_b.end(), str_b.begin(), ::tolower);

  //LOG_DEBUG("operator< was called for string: " << _relativePath << " vs " << val._relativePath << ", transformed: " << str_a << " vs " << str_b);

  return (str_a.compare(str_b) < 0);
}

bool CarrierFile::CompareByPointers(CarrierFile* a, CarrierFile* b) {
  if (a == nullptr)
    return 0;


  if (b == nullptr)
    return 0;

  return (*a < *b);
}

bool CarrierFile::CompareBySharedPointers(std::shared_ptr<CarrierFile> a,
                                          std::shared_ptr<CarrierFile> b) {
  if (a == nullptr)
    return 0;


  if (b == nullptr)
    return 0;

  return (*(a.get()) < *(b.get()));
}


uint64 CarrierFile::GetRawCapacity() {
  return raw_capacity_;
}

void CarrierFile::SetSubkey(const Key& subkey) {
  this->subkey_ = subkey;
}

void CarrierFile::SetBitInBufferPermuted(uint64 index) {
  // TODO: rewrite to run-time exception
  if (index >= permutation_->GetSize()) {
    LOG_INFO("CarrierFile::SetBitInBufferPermuted: index " << index <<
             " is too big!");
    return;
  }

  uint64 permuted_index = permutation_->Permute(index);

  if (permuted_index >= permutation_->GetSize()) {
    LOG_INFO("CarrierFile::SetBitInBufferPermuted: permuted index " << permuted_index << " is too big!");
    return;
  }

  buffer_[permuted_index / 8] |= ( 1 << (permuted_index % 8));
}

uint8 CarrierFile::GetBitInBufferPermuted(uint64 index) {

  if (index >= permutation_->GetSize()) {
    LOG_INFO("CarrierFile::GetBitInBufferPermuted: index " << index <<
             " is too big!");
    return 0;
  }

  uint64 permuted_index = permutation_->Permute(index);

  return ((buffer_[permuted_index / 8] & (1 << (permuted_index % 8))) != 0);
}

int CarrierFile::ExtractBufferUsingEncoder() {
  if (!buffer_.GetSize()) return -1;
  if (!encoder_) return -2;
  if (!codeword_block_size_) return -3;
  if (!blocks_used_) return -4;

  MemoryBuffer data_buffer(data_block_size_);

  for (uint64 b = 0; b < blocks_used_; ++b) {
    encoder_->Extract(&buffer_[b * codeword_block_size_],
        data_buffer.GetRawPointer());

    for (uint64 i = 0; i < data_block_size_; ++i) {
      //TODO:            #warning doriesit try catch!
      try {
        virtual_storage_->WriteByte(virtual_storage_offset_ +
                                    (b * data_block_size_) + i, data_buffer[i]);
      } catch (std::out_of_range& ex) {
        LOG_DEBUG("CarrierFile::extractBufferUsingEncoder: "
                  "virtualStorage->writeByte failed: block: " << (b + 1) << "/"
                  << blocks_used_ << ", byte: " << (i + 1) << "/" <<
                  data_block_size_);
        //TODO: poriesit
        //LOG_TRACE("CarrierFile::extractBufferUsingEncoder: virtualStorage->writeByte failed: error code: " << errc << ", block: " << (b+1) << "/" << blocks_used_ << ", byte: " << (i+1) << "/" << data_block_size_);
        // TODO: poriesit ako zistit kedy je to error a kedy koniec uloziska -> to je ok
        // TODO: virtual_storage_->raw_capacity_ testovat
        //LOG_ERROR("ERROR WRITING BYTE! error code: " << errc << ", block: " << (b+1) << "/" << blocks_used_ << ", byte: " << (i+1) << "/" << data_block_size_);
      }
    }
  }

  return 0;
}


int CarrierFile::EmbedBufferUsingEncoder() {
  //TODO: rewrite using exceptions!
  if (buffer_.GetSize() == 0) return -1;
  if (!encoder_) return -2;
  if (!codeword_block_size_) return -3;
  if (!blocks_used_) return -4;

  MemoryBuffer data_buffer(data_block_size_);

  for (uint64 b = 0; b < blocks_used_; ++b) {
    for (uint64 i = 0; i < data_block_size_; ++i) {
      //TODO:            #warning doriesit try catch!
      //TODO(Matus): poriesit iba na jeden cyklus
      try {
        data_buffer[i] = virtual_storage_->ReadByte(virtual_storage_offset_ +
                                                    (b * data_block_size_) + i);
      } catch (std::out_of_range& ex) {
        LOG_DEBUG("CarrierFile::embedBufferUsingEncoder: virtualStorage->"
                  "readByte failed: block: " << (b + 1) << "/" << blocks_used_
                  << ", byte: " << (i + 1) << "/" << data_block_size_);
        //TODO: poriesit ako zistit kedy je to error a kedy koniec uloziska -> to je ok
        //LOG_ERROR("ERROR READING BYTE! error code: " << errc << ", block: " << (b+1) << "/" << blocks_used_ << ", byte: " << (i+1) << "/" << data_block_size_);
        data_buffer[i] = 0; // TODO: not sure if random data would be better here
      }
    }
    encoder_->Embed(&buffer_[b * codeword_block_size_],
        data_buffer.GetRawPointer());
  }

  return 0;
}

} // stego_disk
