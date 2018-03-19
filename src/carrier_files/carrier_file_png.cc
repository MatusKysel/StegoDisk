/**
* @file carrier_file_bmp.cc
* @author Martin Kosdy
* @author Matus Kysel
* @date 2016
* @brief File with implementation of loading and saving BMP files
*
*/

#include "carrier_file_png.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils/exceptions.h"
#include "utils/stego_errors.h"


namespace stego_disk {

CarrierFilePNG::CarrierFilePNG(File file, std::shared_ptr<Encoder> encoder,
                               std::shared_ptr<Permutation> permutation,
                               std::unique_ptr<Fitness> fitness) :
  CarrierFile(file, encoder, permutation, std::move(fitness)) {

  auto file_ptr = file.Open();

  unsigned char png_header[64];

  fseek(file_ptr.Get(), 0, SEEK_SET);
  int read_cnt = static_cast<int>(fread(&png_header, 1, 64, file_ptr.Get()));


  if (read_cnt < 64) {
	throw exception::ParseError{file_.GetFileName(), "Wrong header size"};
  }

  lodepng_state_init(&state_);
  unsigned error = lodepng_inspect(&width_, &height_, &state_, png_header, 64);
  if(error)
	throw exception::ParseError{file_.GetFileName(), "Unable to read file state"};

  state_.info_raw.colortype = LCT_RGB;
  state_.info_raw.bitdepth = 8;

  raw_capacity_ = (lodepng_get_raw_size(width_, height_, &state_.info_raw) / 8);
}

void CarrierFilePNG::LoadFile() {

  if (file_loaded_) return;

    auto file_ptr = file_.Open();

    LOG_INFO("Loading file " << file_.GetRelativePath());

    if (permutation_->GetSize() == 0) {
      permutation_->Init(raw_capacity_ * 8, subkey_);
    }

    buffer_.Resize(raw_capacity_);
    buffer_.Clear();

    MemoryBuffer png_buffer(file_.GetSize());

    uint64 bits_to_modify = permutation_->GetSize();


    fseek(file_ptr.Get(), 0, SEEK_SET);
    uint32 read_cnt = static_cast<uint32>(fread(png_buffer.GetRawPointer(), 1,
                                                file_.GetSize(),
                                                file_ptr.Get()));

    if (read_cnt < file_.GetSize()) {
      LOG_ERROR("Unable to read file.");
	  throw exception::IoError{file_.GetFileName()};
    }

    unsigned char* image;
    unsigned width, height;

    unsigned error = lodepng_decode(&image, &width, &height, &state_, png_buffer.GetConstRawPointer(), read_cnt);

    if(error)
	  throw exception::ParseError{file_.GetFileName(), "Unable to decode file"};

    // copy LSB data to content buffer

    for (uint64 i = 0; i < bits_to_modify; ++i) {
      if (image[i] & 0x01) SetBitInBufferPermuted(i);
    }

    free(image);

    ExtractBufferUsingEncoder();

    file_loaded_ = true;

    LOG_INFO("File " << file_.GetRelativePath() << " loaded");
}


void CarrierFilePNG::SaveFile() {
  auto file_ptr = file_.Open();

  if(!file_loaded_)
    throw exception::InvalidState{exception::Operation::save,
                                  exception::Component::file,
								  exception::ComponentState::notLoaded};


  LOG_INFO("Saving file " << file_.GetRelativePath());

  if (permutation_->GetSize() == 0) {
    permutation_->Init(raw_capacity_ * 8, subkey_);
  }

  buffer_.Resize(raw_capacity_);
  buffer_.Clear();

  MemoryBuffer png_buffer(file_.GetSize());

  uint64 bits_to_modify = permutation_->GetSize();

  fseek(file_ptr.Get(), 0, SEEK_SET);
  uint32 read_cnt = static_cast<uint32>(fread(png_buffer.GetRawPointer(), 1,
                                              file_.GetSize(),
                                              file_ptr.Get()));

  if (read_cnt < file_.GetSize()) {
    LOG_ERROR("Unable to read file.")
	throw exception::IoError{file_.GetFileName()};
  }
  // copy LSB data to content buffer

  unsigned char* image;
  unsigned width, height;

  unsigned error = lodepng_decode(&image, &width, &height, &state_, png_buffer.GetConstRawPointer(), read_cnt);

  if(error)
    throw exception::ParseError{file_.GetFileName(), "Unable to decode file"};

  // copy LSB data to content buffer

  for (uint64 i = 0; i < bits_to_modify; ++i) {
    if (image[i] & 0x01) SetBitInBufferPermuted(i);
  }

  EmbedBufferUsingEncoder();

  for (uint64 i = 0; i < bits_to_modify ; ++i) {
    image[i] = (image[i] & 0xFE) | GetBitInBufferPermuted(i);
  }

  unsigned char* image_out;
  size_t size_out;

  error = lodepng_encode(&image_out, &size_out, image, width_, height_, &state_);

  if(error)
    throw exception::ParseError{file_.GetFileName(), "Unable to encode file"};

  // write data

  fseek(file_ptr.Get(), 0, SEEK_SET);
  uint32 write_cnt = static_cast<uint32>(fwrite(image_out,
                                                1, size_out,
                                                file_ptr.Get()));

  if (write_cnt != size_out) {
    LOG_ERROR("Writing PNG file expanded");
	throw exception::IoError{file_.GetFileName()};
  }

  free(image);
  free(image_out);

  LOG_INFO("File " << file_.GetRelativePath() << " saved");
}

} // stego_disk
