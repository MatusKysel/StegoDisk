//
//  CarrierFileJPEG.cpp
//  StegoFS
//
//  Created by Martin Kosdy on 1/9/13.
//  Copyright (c) 2013 Martin Kosdy. All rights reserved.
//
#include "carrier_file_jpeg.h"

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include <errno.h>

#include <iostream>

#include "utils/stego_errors.h"
#include "utils/stego_math.h"

#define EXIT_SUCCESS 0

namespace stego_disk {

CarrierFileJPEG::CarrierFileJPEG(File file, std::shared_ptr<Encoder> encoder) :
  CarrierFile(file, encoder) {
  ComputeCapacity();
}

int CarrierFileJPEG::ComputeCapacity() {
  // TODO: test if file is Opened OK
  auto file_ptr = file_.Open();

  LOG_TRACE("Computing capacity of file " << file_.GetSize());

  struct jpeg_decompress_struct cinfo_decompress;
  struct jpeg_error_mgr jerr_decompress;
  jvirt_barray_ptr* coeff_arrays;

  // Create decompression object.
  cinfo_decompress.err = jpeg_std_error(&jerr_decompress);
  jpeg_create_decompress(&cinfo_decompress);

  fseek(file_ptr.Get(), 0, SEEK_SET);
  jpeg_stdio_src(&cinfo_decompress, file_ptr.Get());

  // Get all compression parameters.
  jpeg_read_header(&cinfo_decompress, TRUE);

  // Read coefficients.
  coeff_arrays = jpeg_read_coefficients(&cinfo_decompress);


  JBLOCKARRAY jpeg_block_buffer;
  JCOEFPTR blockptr;
  jpeg_component_info* compptr;

  JDIMENSION ci, by, bx, bi;

  uint32 capacity_in_bits = 0;

  for (ci = 0; ci < 3; ++ci) {
    compptr = cinfo_decompress.comp_info + ci;
    for (by = 0; by < compptr->height_in_blocks; ++by) {
      jpeg_block_buffer = (cinfo_decompress.mem->access_virt_barray)
                          ((j_common_ptr)&cinfo_decompress, coeff_arrays[ci],
                           by, (JDIMENSION)1, FALSE);
      for (bx = 0; bx < compptr->width_in_blocks; ++bx) {
        blockptr = jpeg_block_buffer[0][bx];
        for (bi = 1; bi < 64; ++bi) { // skip the first coeff (DC) - first index is 1 (first AC coeff)
          if (blockptr[bi] & 0xFFFE) ++capacity_in_bits; // ignore coeffs 0,1
        }
      }
    }
  }

  raw_capacity_ = (capacity_in_bits / 8);

  jpeg_finish_decompress(&cinfo_decompress);
  jpeg_destroy_decompress(&cinfo_decompress);

  return 0;

}

int CarrierFileJPEG::LoadFile() {
  if (file_loaded_) return STEGO_NO_ERROR;

  auto file_ptr = file_.Open();
//  if(!file_loaded_) return SE_LOAD_FILE;

  //LOG_TRACE("CarrierFileJPEG::loadFile: Loading file " << file_.GetRelativePath());

  if (permutation_->GetSize() == 0) {
    permutation_->Init(raw_capacity_ * 8, subkey_);
  }

  buffer_.Resize(raw_capacity_);
  buffer_.Clear();

  struct jpeg_decompress_struct cinfo_decompress;
  struct jpeg_error_mgr jerr_decompress;
  jvirt_barray_ptr* coeff_arrays;

  // Create decompression object.
  cinfo_decompress.err = jpeg_std_error(&jerr_decompress);
  jpeg_create_decompress(&cinfo_decompress);

  fseek(file_ptr.Get(), 0, SEEK_SET);
  jpeg_stdio_src(&cinfo_decompress, file_ptr.Get());

  // Get all compression parameters.
  jpeg_read_header(&cinfo_decompress, TRUE);

  // Read coefficients.
  coeff_arrays = jpeg_read_coefficients(&cinfo_decompress);


  JBLOCKARRAY jpeg_block_buffer;
  JCOEFPTR blockptr;
  jpeg_component_info* compptr;

  JDIMENSION ci, by, bx, bi;

  // read data

  uint32 coeff_counter = 0;

  bool should_read = true;

  uint64 bits_to_modify = permutation_->GetSize();

  LOG_TRACE("CarrierFileJPEG::loadFile: file " << file_.GetRelativePath() <<
            ", bits to modify: " << bits_to_modify);

  for (ci = 0; ci < 3; ++ci) {
    compptr = cinfo_decompress.comp_info + ci;
    for (by = 0; (by < compptr->height_in_blocks) && should_read; ++by) {
      jpeg_block_buffer = (cinfo_decompress.mem->access_virt_barray)
                          ((j_common_ptr)&cinfo_decompress, coeff_arrays[ci],
                           by, (JDIMENSION)1, FALSE);
      for (bx = 0; (bx < compptr->width_in_blocks) && should_read; ++bx) {
        blockptr = jpeg_block_buffer[0][bx];
        for (bi = 1; (bi < 64) && should_read; ++bi) { // skip the first coeff (DC) - first index is 1 (first AC coeff)
          if (blockptr[bi] & 0xFFFE) { // ignore 0, 1 coeffs

            if ((blockptr[bi] & 0x1)) SetBitInBufferPermuted(coeff_counter);

            ++coeff_counter;
            if (coeff_counter >= bits_to_modify) should_read = false;

          }
        }
      }
    }
  }

  LOG_TRACE(file_.GetRelativePath() << ", coeff_counter:" << coeff_counter);

  LOG_TRACE(file_.GetRelativePath() << ", unpacked buffer: " <<
            StegoMath::HexBufferToStr(buffer_.GetRawPointer(), 10));

  if (ExtractBufferUsingEncoder()) {
    LOG_ERROR("CarrierFileJPEG::loadFile: file " << file_.GetRelativePath()
              << ", saving: buffer extracting failed!");
  }

  file_loaded_ = true;

  jpeg_finish_decompress(&cinfo_decompress);
  jpeg_destroy_decompress(&cinfo_decompress);

  LOG_TRACE("CarrierFileJPEG::loadFile: file " << file_.GetRelativePath()
            << " loaded");

  return STEGO_NO_ERROR;
}

int CarrierFileJPEG::SaveFile() {

  auto file_ptr = file_.Open();

  LOG_TRACE("Saving file " << file_.GetRelativePath());

  if (permutation_->GetSize() == 0) {
    permutation_->Init(raw_capacity_ * 8, subkey_);
  }

  buffer_.Resize(raw_capacity_);
  buffer_.Clear();

  // JPEG LOADING PHASE -------------------------------------------

  struct jpeg_decompress_struct cinfo_decompress;
  struct jpeg_error_mgr jerr_decompress;
  jvirt_barray_ptr* coeff_arrays;

  // Create decompression object.
  cinfo_decompress.err = jpeg_std_error(&jerr_decompress);
  jpeg_create_decompress(&cinfo_decompress);

  fseek(file_ptr.Get(), 0, SEEK_SET);
  jpeg_stdio_src(&cinfo_decompress, file_ptr.Get());

  // save markers (exif data etc)
  jpeg_save_markers(&cinfo_decompress, JPEG_COM, 0xffff);
  for (int i = 0; i < 16; ++i) {
    jpeg_save_markers(&cinfo_decompress, JPEG_APP0 + i, 0xffff);
  }
  // Get all compression parameters.
  jpeg_read_header(&cinfo_decompress, TRUE);

  // Read coefficients.
  coeff_arrays = jpeg_read_coefficients(&cinfo_decompress);


  // COEF MODIFICATION PHASE ------------------------------------

  JBLOCKARRAY jpeg_block_buffer;
  JCOEFPTR blockptr;
  jpeg_component_info* compptr;

  JDIMENSION ci, by, bx, bi;


  uint32 coeff_counter = 0;

  bool should_write = true;
  bool should_read = true;


  uint32 bits_to_modify = static_cast<uint32>(permutation_->GetSize());

  // LOG_INFO(_relativePath << ", bits to modify: " << bits_to_modify);

  // read LSBs from DCT coefficient and store them in temporary buffer in "locally" permuted order

  for (ci = 0; ci < 3; ++ci) {
    compptr = cinfo_decompress.comp_info + ci;
    for (by = 0; (by < compptr->height_in_blocks) && should_read; ++by) {
      jpeg_block_buffer = (cinfo_decompress.mem->access_virt_barray)
                          ((j_common_ptr)&cinfo_decompress, coeff_arrays[ci],
                           by, (JDIMENSION)1, FALSE);
      for (bx = 0; (bx < compptr->width_in_blocks) && should_read; ++bx) {
        blockptr = jpeg_block_buffer[0][bx];
        for (bi = 1; (bi < 64) && should_read; ++bi) {// skip the first coeff (DC) - first index is 1 (first AC coeff)
          if (blockptr[bi] & 0xFFFE) { // ignore 0, 1 coeffs

            if ((blockptr[bi] & 0x1)) SetBitInBufferPermuted(coeff_counter);

            ++coeff_counter;
            if (coeff_counter >= bits_to_modify) should_read = false;

          }
        }
      }
    }
  }

  //LOG_INFO(_relativePath << ", reading finished coeff_counter:" << coeff_counter);

  // use encoder to embed "globally" permuted bytes of hidden storage to "locally" permuted LSBbits stored in temporary buffer

  //LOG_INFO(_relativePath << ", unpacked buffer: " << StegoMath::hexBufferToStr(buffer_, 10));

  // TODO: what with this error?
  if (EmbedBufferUsingEncoder()) {
    LOG_ERROR("CarrierFileJPEG::saveFile: '" << file_.GetRelativePath() <<
              "': buffer embedding failed!");
  }

  //LOG_INFO(_relativePath << ", embedded buffer: " << StegoMath::hexBufferToStr(buffer_, 10));


  // write down permuted and encoded LSBs into DCT coefficients

  coeff_counter = 0;
//  uint8 tmp_lsb = 0;

  for (ci = 0; ci < 3; ++ci){
    compptr = cinfo_decompress.comp_info + ci;
    for (by = 0; (by < compptr->height_in_blocks) && should_write; ++by) {
      jpeg_block_buffer = (cinfo_decompress.mem->access_virt_barray)
                          ((j_common_ptr)&cinfo_decompress, coeff_arrays[ci],
                           by, (JDIMENSION)1, FALSE);
      for (bx = 0; (bx < compptr->width_in_blocks) && should_write; ++bx) {
        blockptr = jpeg_block_buffer[0][bx];
        for (bi=1; (bi<64) && should_write; bi++)
        {
          if (blockptr[bi] & 0xFFFE) { // ignore 0, 1 coeffs

//            tmp_lsb = GetBitInBufferPermuted(coeff_counter);

            blockptr[bi] = (blockptr[bi] & 0xFFFE) |
                           GetBitInBufferPermuted(coeff_counter);
//            blockptr[bi] |= (tmp_lsb & 0x01);

            coeff_counter++;
            if (coeff_counter >= bits_to_modify) should_write = false;
          }
        }
      }
    }
  }

  // LOG_INFO(_relativePath << ", writing finished coeff_counter:" << coeff_counter);

  // JPEG SAVING PHASE -------------------------------------------

  struct jpeg_compress_struct cinfo_compress;
  struct jpeg_error_mgr jerr_compress;
  jpeg_create_compress(&cinfo_compress);
  cinfo_compress.err = jpeg_std_error(&jerr_compress);
  fseek(file_ptr.Get(), 0, SEEK_SET);
  jpeg_stdio_dest(&cinfo_compress, file_ptr.Get());

  // set jpeg params
  jpeg_copy_critical_parameters(&cinfo_decompress, &cinfo_compress);

  // write coeffs
  jpeg_write_coefficients(&cinfo_compress, coeff_arrays);

  // write markers
  jpeg_saved_marker_ptr marker;
  for(marker = cinfo_decompress.marker_list;
      marker != NULL;
      marker = marker->next) {
    jpeg_write_marker(&cinfo_compress, marker->marker, marker->data,
                      marker->data_length);
  }


  // CLEANUP PHASE -----------------------------------------------

  jpeg_finish_compress(&cinfo_compress);
  jpeg_destroy_compress(&cinfo_compress);

  jpeg_finish_decompress(&cinfo_decompress);
  jpeg_destroy_decompress(&cinfo_decompress);

  LOG_TRACE("CarrierFileJPEG::saveFile: file " << file_.GetRelativePath() <<
            " saved");

  //file_loaded_ = false;

  return STEGO_NO_ERROR;
}

int CarrierFileJPEG::GetHistogram()
{
  /*
    if (loadFile()) return -1;

    JBLOCKARRAY jpeg_block_buffer;
    JCOEFPTR blockptr;
    jpeg_component_info* compptr;

    int ci, by, bx, bi;

    int histogram[0xFFFF];
    memset(histogram, 0, 4*0xFFFF);

    if (_cinfo_decompress.comp_info == NULL) return -1;

    int coefCount = 0;

    for (ci = 0; ci < 3; ci++)
    {
        compptr = _cinfo_decompress.comp_info + ci;
        for (by = 0; by < compptr->height_in_blocks; by++)
        {
            jpeg_block_buffer = (_cinfo_decompress.mem->access_virt_barray)((j_common_ptr)&_cinfo_decompress, _coeff_arrays_one[ci], by, (JDIMENSION)1, FALSE);
            for (bx = 0; bx < compptr->width_in_blocks; bx++)
            {
                blockptr = jpeg_block_buffer[0][bx];
                for (bi = 0; bi < 64; bi++)
                {
                    histogram[(uint16_t)blockptr[bi] ^ (rand() % 2)]++;
                    coefCount++;
                }
            }
        }
    }

    for (short i=-20;i<=20;i++) {
        printf("%7d, %7.3lf\n", (short)i,(double)histogram[(unsigned short)i]*100/(double)coefCount);
    }
    */
  return 0;

}

} // stego_disk
