/**
* @file carrier_file_bmp.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File with implementation of loading and saving BMP files
*
*/

#include "carrier_file_bmp.h"

#include <stdlib.h>
#include <stdio.h>

#include "utils/stego_errors.h"

namespace stego_disk {

CarrierFileBMP::CarrierFileBMP(File file, std::shared_ptr<Encoder> encoder,
                               std::shared_ptr<Permutation> permutation) :
  CarrierFile(file, encoder, permutation) {

  auto file_ptr = file.Open();

  char bmp_header[14];
  char bmp_info[40];

  fseek(file_ptr.Get(), 0, SEEK_SET);
  int read_cnt = static_cast<int>(fread(&bmp_header, 1, 14, file_ptr.Get()));
  read_cnt += static_cast<int>(fread(&bmp_info, 1, 40, file_ptr.Get()));

  if (read_cnt < 54) {
     throw std::runtime_error("Wrong header size of file " + file_.GetFileName());
  }

  uint32_t bmp_file_size = *((uint32_t*)&bmp_header[2]);
  if (bmp_file_size != file_.GetSize()) {
    throw std::runtime_error("Wrong size of file " + file_.GetFileName());
  }

  bmp_offset_ = *((uint32_t*)&bmp_header[10]); // should be = 54
  uint16_t bmp_bits_per_pixel = *((uint16_t*)&bmp_info[14]); // 24
  uint32_t bmp_compression = *((uint32_t*)&bmp_info[16]); // should be 0

  if (bmp_compression != 0) {
    throw std::runtime_error("BMP file " + file_.GetFileName() +
                             " uses unsupported file compression");
  }

  //  if (bmp_bits_per_pixel != 24) {
  //    //TODO: throw exception
  //    return;
  //  } TODO(Matus) podpora inych hlbok

  //bmp_size_ = *((uint32_t*)&bmp_info[20]); // byva = 0 pri vypnutej kompresii

  int32_t bmp_width = *((int32_t*)&bmp_info[4]);
  int32_t bmp_height = *((int32_t*)&bmp_info[8]);

  bmp_size_ = (((bmp_bits_per_pixel * bmp_width + 31) / 32) * 4) *
              abs(bmp_height);

  if ((bmp_size_ + 54) > bmp_file_size) {
    throw std::runtime_error("Error occured while reading file "
                             + file_.GetFileName());
  }
  raw_capacity_ = (bmp_size_ / 8);
}



int CarrierFileBMP::LoadFile() {

  if (file_loaded_) return STEGO_NO_ERROR;

  auto file_ptr = file_.Open();

  LOG_INFO("Loading file " << file_.GetRelativePath());

  if (permutation_->GetSize() == 0) {
    permutation_->Init(raw_capacity_ * 8, subkey_);
  }

  buffer_.Resize(raw_capacity_);
  buffer_.Clear();

  MemoryBuffer bitmap_buffer(raw_capacity_ * 8);

  uint64 bits_to_modify = permutation_->GetSize();


  fseek(file_ptr.Get(), bmp_offset_, SEEK_SET);
  uint32 read_cnt = static_cast<uint32>(fread(bitmap_buffer.GetRawPointer(), 1,
                                              raw_capacity_ * 8,
                                              file_ptr.Get()));

  if (read_cnt != raw_capacity_ * 8) {
    LOG_ERROR("Unable to read file.");
    throw std::runtime_error("Unable to read to read file " + file_.GetFileName());
  }

  // copy LSB data to content buffer

  for (uint64 i = 0; i < bits_to_modify; ++i) {
    if (bitmap_buffer[i] & 0x01) SetBitInBufferPermuted(i);
  }

  ExtractBufferUsingEncoder();

  file_loaded_ = true;

  LOG_INFO("File " << file_.GetRelativePath() << " loaded");

  return STEGO_NO_ERROR;
}


int CarrierFileBMP::SaveFile() {
  auto file_ptr = file_.Open();

  if(!file_loaded_) throw std::runtime_error("File " + file_.GetFileName() +
                                             " is not loaded");


  LOG_INFO("Saving file " << file_.GetRelativePath());

  if (permutation_->GetSize() == 0) {
    permutation_->Init(raw_capacity_ * 8, subkey_);
  }

  buffer_.Resize(raw_capacity_);
  buffer_.Clear();

  MemoryBuffer bitmap_buffer(raw_capacity_ * 8);

  uint64 bits_to_modify = permutation_->GetSize();

  fseek(file_ptr.Get(), bmp_offset_, SEEK_SET);
  uint32 read_cnt = static_cast<uint32>(fread(bitmap_buffer.GetRawPointer(), 1,
                                              raw_capacity_ * 8,
                                              file_ptr.Get()));

  if (read_cnt != raw_capacity_ * 8) {
    LOG_ERROR("Unable to read file.")
    throw std::runtime_error("Unable to read to read file " + file_.GetFileName());
  }
  // copy LSB data to content buffer

  for (uint64 i = 0; i < bits_to_modify; ++i) {
    if (bitmap_buffer[i] & 0x01) SetBitInBufferPermuted(i);
  }

  EmbedBufferUsingEncoder();

  for (uint64 i = 0; i < bits_to_modify ; ++i) {
    bitmap_buffer[i] = (bitmap_buffer[i] & 0xFE) | GetBitInBufferPermuted(i);
  }

  // write data

  fseek(file_ptr.Get(), bmp_offset_, SEEK_SET);
  uint32 write_cnt = static_cast<uint32>(fwrite(bitmap_buffer.GetRawPointer(),
                                                1, raw_capacity_ * 8,
                                                file_ptr.Get()));

  if (write_cnt != raw_capacity_ * 8) {
    LOG_ERROR("Writing content to file failed.");
    throw std::runtime_error("Writing content to file " + file_.GetFileName() +
                             " failed");
  }

  LOG_INFO("File " << file_.GetRelativePath() << " saved");

  return STEGO_NO_ERROR;
}

} // stego_disk
